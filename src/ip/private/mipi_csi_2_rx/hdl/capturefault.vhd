-- Retain receive-domain errors before crossing to the video domain.
-- Pulse inputs must span a receive clock edge. Clear only with the coordinated
-- CSI/video/capture session reset; this is a fault channel, not a frame token.
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity CaptureFault is
   port (
      rx_clk, rx_reset, video_clk, video_reset : in std_logic;
      rx_fault : in std_logic_vector(2 downto 0);
      video_fault : out std_logic_vector(2 downto 0)
   );
end CaptureFault;

architecture Behavioral of CaptureFault is
   signal rx_sticky : std_logic_vector(2 downto 0) := (others => '0');
begin
   process(rx_clk, rx_reset)
   begin
      if rx_reset = '1' then
         rx_sticky <= (others => '0');
      elsif rising_edge(rx_clk) then
         rx_sticky <= rx_sticky or rx_fault;
      end if;
   end process;
   fault_sync: for i in 0 to 2 generate
      SyncAsync_capture: entity work.SyncAsync
         port map(aReset => video_reset, aIn => rx_sticky(i),
                  OutClk => video_clk, oOut => video_fault(i));
   end generate;
end Behavioral;
