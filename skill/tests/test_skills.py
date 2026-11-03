import copy
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

ROOT=Path(__file__).resolve().parents[1]
def load(name,path):
    spec=importlib.util.spec_from_file_location(name,ROOT/path)
    m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);return m
gold=load('gold','hls-golden/scripts/compare_json.py')
hls=load('hls','hls-golden/scripts/inspect_hls.py')
reports=load('reports','vivado-report-gate/scripts/check_reports.py')
release=load('release','release-provenance/scripts/verify_manifest.py')
vision=load('vision','vision-motion-contract/scripts/preview_task.py')
pynq=load('pynq_meta','pynq-overlay-portability/scripts/preflight.py')


class GoldenTests(unittest.TestCase):
    def test_exact(self):self.assertTrue(gold.compare({'a':[0,255,-1],'status':0},{'a':[0,255,-1],'status':0})['passed'])
    def test_changed_pixel(self):self.assertFalse(gold.compare([0,255],[0,254])['passed'])
    def test_missing_key(self):self.assertFalse(gold.compare({'status':0},{})['passed'])
    def test_wrong_length(self):self.assertFalse(gold.compare([1,2],[1])['passed'])
    def test_bool_not_number(self):self.assertFalse(gold.compare(True,1)['passed'])
    def test_tolerance(self):self.assertTrue(gold.compare([1.0],[1.001],atol=.002)['passed'])
    def test_nan_rejected(self):self.assertFalse(gold.compare([float('nan')],[float('nan')])['passed'])
    def test_empty_not_evidence(self):self.assertFalse(gold.compare([],[])['passed'])
    def test_negative_tolerance(self):
        with self.assertRaises(ValueError):gold.compare([1],[1],atol=-1)
    def test_large_integer_exact(self):self.assertFalse(gold.compare(2**63,2**63+1)['passed'])
    def test_historical_oracle_shape(self):
        data=json.loads((ROOT/'evidence/e05_color_oracle.json').read_text())
        changed=copy.deepcopy(data);changed['cases'][0]['details'][0]['votes'][4]+=1
        result=gold.compare(data,changed)
        self.assertFalse(result['passed']);self.assertEqual(result['mismatch_count'],1)


class HLSTests(unittest.TestCase):
    def setUp(self):
        self.xml=(ROOT/'evidence/e03_color_csynth.xml').read_text()
        self.cosim=(ROOT/'evidence/e04_color_cosim.rpt').read_text()
    def test_actual_color_reports(self):
        out=hls.inspect(self.xml,self.cosim)
        self.assertTrue(out['passed']);self.assertEqual(out['hls_estimates']['resources']['LUT'],2678)
    def test_cosim_fail(self):self.assertFalse(hls.inspect(self.xml,self.cosim.replace('Pass','Fail'))['passed'])
    def test_stdout_marker_not_enough(self):
        with self.assertRaises(ValueError):hls.inspect(self.xml,'PASS all tests')
    def test_stale_receipt(self):
        out=hls.inspect(self.xml,self.cosim,{'rtl_cosim_pass':False})
        self.assertTrue(out['rtl_report_pass']);self.assertFalse(out['passed'])
    def test_real_historical_contradiction(self):
        xml=(ROOT/'evidence/e08_components_csynth.xml').read_text()
        cosim=(ROOT/'evidence/e07_components_cosim.rpt').read_text()
        receipt=json.loads((ROOT/'evidence/e06_components_old_receipt.json').read_text())
        result=hls.inspect(xml,cosim,receipt)
        self.assertTrue(result['rtl_report_pass']);self.assertFalse(result['passed']);self.assertTrue(result['receipt_conflicts'])


class ReportTests(unittest.TestCase):
    def setUp(self):
        self.args=[(ROOT/('evidence/'+n)).read_text() for n in ['e12_timing.rpt','e13_check_timing.rpt','e14_drc.rpt','e15_utilization.rpt']]
    def test_real_reports_rejected(self):
        r=reports.check(*self.args)
        self.assertFalse(r['passed']);self.assertEqual(r['metrics']['timing']['wns_ns'],-.247);self.assertEqual(r['metrics']['timing']['whs_ns'],-1.111)
        self.assertEqual(r['metrics']['utilization']['lut']['used'],21857)
    def test_missing_report_fails(self):self.assertFalse(reports.check('','','','')['passed'])
    def test_unconstrained_detected(self):
        c=self.args[1].replace('There are 0 pins that are not constrained for maximum delay.','There are 2 pins that are not constrained for maximum delay.')
        self.assertEqual(reports.parse_check(c)['unconstrained_internal'],2)
    def test_resource_overflow(self):
        out=reports.check(*self.args,max_utilization=.1)
        self.assertTrue(any('utilization exceeds' in x for x in out['issues']))
    def test_drc_unknown_severity(self):
        with self.assertRaises(ValueError):reports.parse_drc(self.args[2].replace('| Warning  |','| UNKNOWN  |'))
    def test_positive_fixture(self):
        timing=' WNS(ns) TNS(ns) TNS Failing Endpoints TNS Total Endpoints WHS(ns) THS(ns) THS Failing Endpoints THS Total Endpoints WPWS(ns) TPWS(ns) TPWS Failing Endpoints TPWS Total Endpoints\n -----\n 0.2 0 0 10 0.1 0 0 10 0.2 0 0 10\n'
        coverage=self.args[1].replace('There are 6 input ports','There are 0 input ports').replace('There are 9 ports','There are 0 ports')
        out=reports.check(timing,coverage,self.args[2],self.args[3])
        self.assertTrue(out['passed']);self.assertFalse(out['full_signoff'])


class ManifestTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup);self.root=Path(self.temp.name)
        (self.root/'a.txt').write_bytes(b'one');(self.root/'b.txt').write_bytes(b'two')
        a=hashlib.sha256(b'one').hexdigest();b=hashlib.sha256(b'two').hexdigest()
        self.manifest={'schema_version':1,'required_roles':['hardware','application'],'artifacts':[{'id':'a','role':'hardware','path':'a.txt','sha256':a,'depends_on':{}},{'id':'b','role':'application','path':'b.txt','sha256':b,'depends_on':{'a':a}}]}
    def test_valid(self):self.assertTrue(release.verify(self.root,self.manifest)['passed'])
    def test_changed_file(self):
        (self.root/'a.txt').write_bytes(b'changed');self.assertFalse(release.verify(self.root,self.manifest)['passed'])
    def test_bad_parent(self):
        self.manifest['artifacts'][1]['depends_on']['a']='0'*64;self.assertFalse(release.verify(self.root,self.manifest)['passed'])
    def test_missing_file(self):
        (self.root/'b.txt').unlink();self.assertFalse(release.verify(self.root,self.manifest)['passed'])
    def test_escape_path(self):
        self.manifest['artifacts'][0]['path']='../a.txt';self.assertFalse(release.verify(self.root,self.manifest)['passed'])
    def test_windows_absolute_rejected(self):
        self.manifest['artifacts'][0]['path']='C:\\Windows\\a.txt';self.assertFalse(release.verify(self.root,self.manifest)['passed'])
    def test_cycle(self):
        self.manifest['artifacts'][0]['depends_on']={'b':self.manifest['artifacts'][1]['sha256']};self.assertFalse(release.verify(self.root,self.manifest)['passed'])
    def test_missing_role(self):
        self.manifest['required_roles'].append('boot');self.assertFalse(release.verify(self.root,self.manifest)['passed'])


class VisionTests(unittest.TestCase):
    def setUp(self):
        self.config=json.loads((ROOT/'vision-motion-contract/examples/config.json').read_text())
        self.result=json.loads((ROOT/'vision-motion-contract/examples/frame.json').read_text())
    def test_transform(self):
        r=vision.preview(self.result,self.config)
        self.assertTrue(r['passed']);self.assertEqual(r['candidates'][0]['x_cm'],'1.25');self.assertEqual(r['candidates'][0]['y_cm'],'-2.50')
    def test_range_rejected_before_rounding(self):
        self.result['objects'][0]['center_cm'][0]=13.001;self.assertFalse(vision.preview(self.result,self.config)['passed'])
    def test_mixed_generation(self):
        self.result['objects'][0]['generation']=8;self.assertFalse(vision.preview(self.result,self.config)['passed'])
    def test_stale(self):
        self.result['age_ms']=10000;self.assertFalse(vision.preview(self.result,self.config)['passed'])
    def test_unknown_shape(self):
        self.result['objects'][0]['shape']='UNK';self.assertFalse(vision.preview(self.result,self.config)['passed'])
    def test_batch_atomic_rejection(self):
        bad=copy.deepcopy(self.result['objects'][0]);bad['valid']=False;self.result['objects'].append(bad)
        r=vision.preview(self.result,self.config);self.assertFalse(r['passed']);self.assertEqual(r['candidates'],[])
    def test_zero_objects(self):
        self.result['objects']=[];r=vision.preview(self.result,self.config);self.assertTrue(r['passed']);self.assertEqual(r['candidates'],[])
    def test_bool_coordinate(self):
        self.result['objects'][0]['center_cm'][0]=True;self.assertFalse(vision.preview(self.result,self.config)['passed'])
    def test_units(self):
        self.result['units']='pixel'
        with self.assertRaises(ValueError):vision.preview(self.result,self.config)


class PYNQTests(unittest.TestCase):
    def setUp(self):
        self.temp=tempfile.TemporaryDirectory();self.addCleanup(self.temp.cleanup);self.root=Path(self.temp.name)
        self.bit=self.root/'demo.bit';self.bit.write_bytes(b'OFFLINE TEST FIXTURE NOT A BITSTREAM')
        self.hwh=self.root/'demo.hwh';self.hwh.write_text('<SYSTEM><MODULES><MODULE FULLNAME="/accel_0" VLNV="example.org:hls:add:1.0"><REGISTERS><REGISTER NAME="CTRL" ADDRESS_OFFSET="0x00"/><REGISTER NAME="length"><PROPERTIES><PROPERTY NAME="ADDRESS_OFFSET" VALUE="0x10"/></PROPERTIES></REGISTER></REGISTERS></MODULE></MODULES></SYSTEM>')
        self.contract={'required_ips':{'accel_0':{'vlnv':'example.org:hls:add:1.0','registers':{'CTRL':'0x00','length':'0x10'}}}}
    def test_valid_metadata(self):self.assertTrue(pynq.inspect(self.bit,self.hwh,self.contract)['passed'])
    def test_missing_ip(self):
        self.contract['required_ips']['other_0']={};self.assertFalse(pynq.inspect(self.bit,self.hwh,self.contract)['passed'])
    def test_offset_mismatch(self):
        self.contract['required_ips']['accel_0']['registers']['length']='0x18';self.assertFalse(pynq.inspect(self.bit,self.hwh,self.contract)['passed'])
    def test_hash_mismatch(self):
        self.contract['expected_sha256']={'bit':'0'*64};self.assertFalse(pynq.inspect(self.bit,self.hwh,self.contract)['passed'])
    def test_rename_contract_another_topic(self):
        self.hwh.write_text(self.hwh.read_text().replace('accel_0','filter_2').replace('add:1.0','filter:2.0'))
        self.contract={'required_ips':{'filter_2':{'vlnv':'example.org:hls:filter:2.0','registers':{'CTRL':0}}}}
        self.assertTrue(pynq.inspect(self.bit,self.hwh,self.contract)['passed'])
    def test_stem_mismatch(self):
        other=self.root/'other.bit';other.write_bytes(b'fixture');self.assertFalse(pynq.inspect(other,self.hwh,self.contract)['passed'])
    def test_bad_xml(self):
        self.hwh.write_text('<broken')
        with self.assertRaises(Exception):pynq.inspect(self.bit,self.hwh,self.contract)


class CLITests(unittest.TestCase):
    def test_json_cli_exit_codes(self):
        with tempfile.TemporaryDirectory() as t:
            g=Path(t)/'g.json';a=Path(t)/'a.json';g.write_text('[1,2]');a.write_text('[1,3]')
            cmd=[sys.executable,str(ROOT/'hls-golden/scripts/compare_json.py'),'--golden',str(g),'--actual',str(a)]
            run=subprocess.run(cmd,capture_output=True,text=True)
            self.assertEqual(run.returncode,1);self.assertFalse(json.loads(run.stdout)['passed'])
            a.write_text('[1,2]');run=subprocess.run(cmd,capture_output=True,text=True)
            self.assertEqual(run.returncode,0);self.assertTrue(json.loads(run.stdout)['passed'])
    def test_preview_cli_no_hardware(self):
        cmd=[sys.executable,str(ROOT/'vision-motion-contract/scripts/preview_task.py'),'--result',str(ROOT/'vision-motion-contract/examples/frame.json'),'--config',str(ROOT/'vision-motion-contract/examples/config.json')]
        run=subprocess.run(cmd,capture_output=True,text=True)
        self.assertEqual(run.returncode,0,run.stderr);self.assertFalse(json.loads(run.stdout)['hardware_access'])


if __name__=='__main__':unittest.main()
