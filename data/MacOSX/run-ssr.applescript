-- To store last chosen renderer type
property previousSelection : 1

on run argv
	-- Get absolute path to SSR binary via args
	-- For relative paths to work, the binary must be placed in MacOS/ folder of .app bundle
	set ssrBinPath to first item of argv
	set argv to rest of argv

	-- Make list of renderer type names and their parameter equivalents
	set rendererNames to {"Binaural (using HRIRs)", "Binaural Room Synthesis (using BRIRs)", "Stereophonic (Vector Base Amplitude Panning)", "Wave Field Synthesis", "Ambisonics Amplitude Panning", "Distance-Coded Ambisonics (experimental)", "Generic Renderer"}
	set rendererOptions to {"--binaural", "--brs", "--vbap", "--wfs", "--aap", "--dca", "--generic"}

	-- Process other given command line options
	set pickRendererType to true
	set options to "" as Unicode text
	repeat with arg in argv
		set options to options & " " & arg
		if arg is in rendererOptions then
			set pickRendererType to false
		end if
	end repeat

	-- Let user pick renderer type if none was given in argv
	set rendererOption to "" as Unicode text
	if (pickRendererType) then
		if previousSelection is greater than (count of rendererNames) then
			set previousSelection to 1
		end if
		tell application "System Events"
			activate
			set selectedRendererName to choose from list rendererNames with title "Start SoundScape Renderer" with prompt "Please make sure Jack is running. Select the type of renderer for this session:" default items {item previousSelection of rendererNames}
		end tell
		if selectedRendererName is false then
			return
		else
			set selectedRendererName to first item of selectedRendererName
			repeat with i from 1 to (count of items in rendererNames)
				if selectedRendererName is equal to (item i of rendererNames) then
					set rendererOption to item i of rendererOptions
					-- Save user choice
					set previousSelection to i
					exit repeat
				end if
			end repeat
		end if
	end if

	-- Assemble shell command string
	set command to "export ECASOUND=\"" & ssrBinPath & "/ecasound\" ; cd \"" & ssrBinPath & "/../../..\" ; \"" & ssrBinPath & "/ssr\" " & rendererOption & options & " && (echo SSR quit normally with exit code $?) || (echo SSR encountered an error and had to quit. See above.  Exit code: $?)"

	-- Open new Terminal window, cd to SSR working dir and start SSR
	tell application "System Events" to set terminalRunning to (exists process "Terminal")
	tell application "Terminal"
		if terminalRunning then
			activate
			set ssrTab to do script command
		else -- Terminal not running yet. Start it and use fresh default window
			activate
			set ssrTab to do script command in front window
		end if

		-- Position Terminal window in upper left corner
		set position of front window to {0, 0}

		-- Wait until SSR quits
		delay 2
		repeat until (ssrTab's history contains command)
			delay 1
		end repeat
		repeat while (ssrTab is busy)
			delay 1
		end repeat

		-- Close Terminal window if SSR did quit with exit code 0
		ignoring case
			if (ssrTab's history contains "SSR quit normally with exit code 0") and (ssrTab's history does not contain "--help") then
				close (first window whose selected tab is ssrTab) saving no
			end if
		end ignoring
	end tell

end run
