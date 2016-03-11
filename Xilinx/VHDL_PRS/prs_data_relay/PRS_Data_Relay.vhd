----------------------------------------------------------------------------------
-- Company: Capitol Technology University
-- Engineer: Keegan Moore & Zach Richard
-- 
-- Create Date:    18:46:56 03/09/2016 
-- Design Name: Parallel Read Strobe (PRS) Data Relay
-- Module Name:    PRS_Data_Relay - PRS_Data_Relay_Behavioral 
-- Project Name: TRAPSat RockSat-X 2016
-- Target Devices: Nexsys3 Spartan-6 
-- Tool versions: 14.7(nt64)
-- Description: 
--
-- Dependencies: 
--
-- Revision: 
-- Revision 0.01 - File Created
-- Additional Comments: 
--
----------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--use IEEE.NUMERIC_STD.ALL;

-- Uncomment the following library declaration if instantiating
-- any Xilinx primitives in this code.
--library UNISIM;
--use UNISIM.VComponents.all;



entity PRS_Data_Relay is
    Port ( clock : in STD_LOGIC;
			  prs : out  STD_LOGIC;
           parallel_bus : inout  STD_LOGIC_VECTOR (7 downto 0);
           serial_port : inout  STD_LOGIC);
end PRS_Data_Relay;

architecture PRS_Data_Relay_Behavioral of PRS_Data_Relay is

signal counter_1usec : std_logic_vector(22 downto 0);
signal state : std_logic_vector(6 downto 0);
signal reset : bit := '0';
signal prs_sig : STD_LOGIC := '0';

begin

-- bleebledoobleblopblopdbleedblopdabloobop 

clock_1usec : process (clock) 
begin
	--if (reset = '1') then
	--	counter_1usec <= "00000000000000000000000";
	--end if;
	if rising_edge(clock) then 
		counter_1usec <= counter_1usec + '1';
	end if;   
end process;

count_1usec : process (counter_1usec(22)) 
begin
	if (state = "1101001") then 
		-- d'105 -> 0
		state <= "0000000";
	elsif (state = "0000000") then
		-- set prs for 1us
		prs_sig <= '1';
	else
		-- clear prs for 105us
		prs_sig <= '0';
		state <= state + '1';
	end if;
	prs <= prs_sig;
end process;

--prs_loop : process (counter(22))
--begin
--	if rising_edge(counter(22)) then
--		prs_sig <= not prs_sig;
--		prs <= prs_sig;
--		reset <= '1';
--	end if;
--	reset <= '0';
--end process;


end PRS_Data_Relay_Behavioral;

