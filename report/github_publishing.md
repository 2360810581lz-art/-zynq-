# GitHub 发布说明

仓库根目录就是 autonomous_sorting，README.md 与 src、skill、report 等目录直接放在根目录。不要把外层的压缩包、audit 目录和压缩校验记录一起提交。

## 这份副本改了什么

在比赛交付版上增加 .gitignore、.gitattributes 和本说明。完整性检查现在跳过 Git 自己的 .git 目录，其余文件仍逐个核对。manifest.json 随这几处文档和脚本调整重新生成。控制源码、坐标表和配套镜像没有修改。

.gitattributes 关闭自动换行转换。这个工程对源码和镜像使用字节哈希，克隆时改变 CRLF/LF 会让校验失败。不要在没有检查影响的情况下开启自动格式化或批量转换换行。

.gitignore 只过滤本地缓存、新输出和根目录压缩包。历史验证日志、bit、HDF、ELF 和 BOOT 是复现材料，仍然保留。

## 公开内容与许可

本仓库保留完整交付材料，包括 report/collaboration/records 中选取的 77 条历史消息，以及板卡资料和配套镜像。来源路径保留在记录中，便于追溯。

第三方 IP、参考设计和资料继续受各自原有许可约束，详见 [第三方说明](third_party_notices.md)。原交付包没有统一的全工程开源授权，这份副本也没有新增统一许可证。公开仓库不等于其中每个文件都可以自由再分发或重新授权；原创代码的许可证需要在确认权属后单独确定。

## 仓库地址与日常更新

公开仓库地址是 https://github.com/2360810581lz-art/-zynq- ，默认分支为 main。首次发布沿用远端原有的初始化提交。作者邮箱使用 GitHub 的 noreply 地址。

后续修改在发布副本中进行。确认离线检查通过、manifest.json 已随实际修改更新，再提交和推送：

```powershell
git status
git add --all
git commit -m "Update project sources and documentation"
git push origin main
```

若 Git 提示登录，在 Git Credential Manager 打开的页面完成；不要把密码或访问令牌写到脚本、README 或远端地址里。

## 上传后

先确认 GitHub 首页能显示项目 README，再检查 src、skill、report 和 board 目录。按根 README 中的离线命令验证克隆结果。完整性检查应先于会更新收据的回放脚本执行。
