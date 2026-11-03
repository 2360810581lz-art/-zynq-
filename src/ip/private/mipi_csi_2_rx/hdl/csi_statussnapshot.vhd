library ieee;
use ieee.std_logic_1164.all;

library xpm;
use xpm.vcomponents.all;

entity CSI_StatusSnapshot is
  port (
    src_clk              : in  std_logic;
    dst_clk              : in  std_logic;
    src_rst              : in  std_logic;
    dst_rst              : in  std_logic;
    src_frame_count      : in  std_logic_vector(31 downto 0);
    src_crc_error_count  : in  std_logic_vector(31 downto 0);
    dst_frame_count      : out std_logic_vector(31 downto 0);
    dst_crc_error_count  : out std_logic_vector(31 downto 0)
  );
end entity;

architecture rtl of CSI_StatusSnapshot is
  signal source_payload  : std_logic_vector(63 downto 0) := (others => '0');
  signal source_send     : std_logic := '0';
  signal source_received : std_logic;
  signal destination_req : std_logic;
  signal destination_data: std_logic_vector(63 downto 0);
  signal source_state    : natural range 0 to 2 := 0;
  signal dst_payload     : std_logic_vector(63 downto 0) := (others => '0');
begin
  -- The payload is CRC in the upper half and FrameCount in the lower half.
  -- Hold through the entire four-phase handshake, including functional resets.
  -- XPM has no reset port: resetting only our state can corrupt an in-flight
  -- exchange. Compare the latest pair again whenever idle; no pending queue is
  -- needed, and an update on acknowledgement release cannot be lost.
  SourceMailbox: process(src_clk)
    variable current_payload : std_logic_vector(63 downto 0);
  begin
    if rising_edge(src_clk) then
      current_payload := src_crc_error_count & src_frame_count;
      if src_rst = '1' then
        current_payload := (others => '0');
      end if;
      case source_state is
        when 0 =>
          if current_payload /= source_payload then
            source_payload <= current_payload;
            source_send <= '1';
            source_state <= 1;
          end if;
        when 1 =>
          if source_received = '1' then
            source_send <= '0';
            source_state <= 2;
          end if;
        when 2 =>
          if source_received = '0' then source_state <= 0; end if;
      end case;
    end if;
  end process;

  StatusHandshake: xpm_cdc_handshake
    generic map (
      DEST_EXT_HSK => 0,
      DEST_SYNC_FF => 3,
      SRC_SYNC_FF  => 3,
      INIT_SYNC_FF => 1,
      SIM_ASSERT_CHK => 1,
      WIDTH => 64)
    port map (
      src_clk  => src_clk,
      src_in   => source_payload,
      src_send => source_send,
      src_rcv  => source_received,
      dest_clk => dst_clk,
      dest_out => destination_data,
      dest_req => destination_req,
      dest_ack => '0');

  DestinationMailbox: process(dst_clk)
  begin
    if rising_edge(dst_clk) then
      if dst_rst = '1' then
        dst_payload <= (others => '0');
      else
        -- dest_out is already an atomic destination-domain XPM register.
        -- Restore it after a local reset even when no new source update occurs.
        dst_payload <= destination_data;
      end if;
    end if;
  end process;

  dst_crc_error_count <= dst_payload(63 downto 32);
  dst_frame_count     <= dst_payload(31 downto 0);
end architecture;
