-- Pure Data (Pd) external for remote-controlling the SoundScape Renderer (SSR)
-- Matthias Geier, Sept. 2014

local SsrClient = pd.Class:new():register("ssrclient")

-- XML parser from http://github.com/Phrogz/SLAXML
local SLAXML = require("slaxml")

function SsrClient:output_command()
    -- first part of command becomes the selector, the rest atoms
    self:outlet(1, self.command[1], {select(2, unpack(self.command))})
    self.command = {}
end

function SsrClient:backup_command()
    self.command_backup = {}
    for k, v in ipairs(self.command) do
        self.command_backup[k] = v
    end
end

function SsrClient:restore_command()
    self.command = {}
    for k, v in ipairs(self.command_backup) do
        self.command[k] = v
    end
end

-- init stuff and set up callbacks for XML parser
function SsrClient:initialize(name, atoms)
    self.inlets = 2
    self.outlets = 2
    self.buffer = {}
    self.command = {}

    self.parser = SLAXML:parser{
        startElement = function(name, nsURI, nsPrefix)
            if name == "update" then
                -- not checked
            elseif name == "source" then
                self.command = {"src"}
            elseif name == "reference" then
                self.command = {"ref"}
                self:backup_command()
            elseif name == "loudspeaker" then
                self.command = {"ls"}
                self:backup_command()
            elseif name == "transport" then
                self.command = {"transport"}
            elseif name == "volume" then
                table.insert(self.command, "vol")
            elseif name == "file" then
                self:restore_command()
                table.insert(self.command, "file")
            elseif name == "port" then
                self:restore_command()
                table.insert(self.command, "port")
            elseif name == "position" then
                self:restore_command()
                table.insert(self.command, "pos")
            elseif name == "orientation" then
                self:restore_command()
                table.insert(self.command, "azi")
            elseif name == "state" or name == "scene" then
                -- these strings don't become part of the message
                self:backup_command()
            else
                self:error(name .. " is ignored")
            end
        end,
        attribute = function(name, value, nsURI, nsPrefix)
            if name == "id" then
                table.insert(self.command, tonumber(value))
                self:backup_command()
            elseif name == "x" then
                table.insert(self.command, tonumber(value))
            elseif name == "y" then
                table.insert(self.command, tonumber(value))
                self:output_command()
            elseif name == "azimuth" then
                table.insert(self.command, tonumber(value))
                self:output_command()
            elseif name == "name" or name == "model" or name == "transport" then
                self:restore_command()
                table.insert(self.command, name)
                table.insert(self.command, value)
                self:output_command()
            elseif name == "mute" or name == "fixed" then
                self:restore_command()
                table.insert(self.command, name)
                if value == "true" or value == "1" then
                    table.insert(self.command, 1)
                elseif value == "false" or value == "0" then
                    table.insert(self.command, 0)
                else
                    self:error("invalid value for mute: " .. value)
                end
                self:output_command()
            elseif name == "length" or name == "file_length" or
                   name == "level" then
                self:restore_command()
                table.insert(self.command, name)
                table.insert(self.command, tonumber(value))
                self:output_command()
            elseif name == "volume" then
                self:restore_command()
                table.insert(self.command, "vol")
                table.insert(self.command, tonumber(value))
                self:output_command()
            elseif name == "channel" then
                self:restore_command()
                table.insert(self.command, "file_channel")
                table.insert(self.command, tonumber(value))
                self:output_command()
                self:restore_command()
                table.insert(self.command, "file")
            elseif name == "output_level" then
                self:restore_command()
                table.insert(self.command, "weights")
                for weight in value:gmatch("%S+") do
                    table.insert(self.command, tonumber(weight))
                end
                self:output_command()
            else
                self:error("ignored attribute: " .. name .. " value: " .. value)
            end
        end,
        closeElement = function(name, nsURI)
            -- nothing
        end,
        text = function(text)
            if self.command[1] == "vol" then
                table.insert(self.command, tonumber(text))
            else
                table.insert(self.command, text)
            end
            self:output_command()
        end,
        comment = function(content)
            self:error("No comment allowed in XML string")
        end,
        pi = function(target, content)
            self:error("No processing instructions allowed in XML string")
        end,
    }
    return true
end

-- create XML string and send it out as list of ASCII numbers
function SsrClient:in_1(sel, atoms)
    local str = '<request>'
    if sel == "src" then
        str = str .. '<source id="' .. atoms[1] .. '"'
        local subcommand = atoms[2]
        if subcommand == "pos" then
            str = str .. '><position x="' .. atoms[3] ..
                                  '" y="' .. atoms[4] .. '"/></source>'
        elseif subcommand == "azi" then
            str = str .. '><orientation azimuth="' .. atoms[3] .. '"/></source>'
        elseif subcommand == "model" then
            str = str .. ' model="' .. atoms[3] .. '"/>'
        elseif subcommand == "mute" then
            local mute_str
            if atoms[3] == 0 then
                mute_str = "false"
            else
                mute_str = "true"
            end
            str = str .. ' mute="' .. mute_str .. '"/>'
        elseif subcommand == "gain" then
            self:error('"gain" not supported, use "vol"')
            return
        else
            self:error(subcommand .. " not supported")
            return
        end
    elseif sel == "ref" then
        str = str .. '<reference'
        local subcommand = atoms[1]
        if subcommand == "pos" then
            str = str .. '><position x="' .. atoms[2] ..
                                  '" y="' .. atoms[3] .. '"/></reference>'
        elseif subcommand == "azi" then
            str = str .. '><orientation azimuth="' .. atoms[2] ..
                  '"/></reference>'
        else
            self:error(subcommand .. " not supported")
            return
        end
    elseif sel == "state" then
        str = str .. '<state ' .. atoms[1] .. '="' .. atoms[2] .. '"/>'
    else
        self:error(sel .. " not (yet?) supported")
        return
    end
    str = str .. '</request>\0'  -- terminated with a binary zero
    self:outlet(2, "list", {str:byte(1, #str)})  -- convert to ASCII numbers
end

-- collect numbers in self.buffer. If a zero comes in, parse the whole string.
function SsrClient:in_2_float(f)
    if f == 0 then
        self.parser:parse(table.concat(self.buffer), {stripWhitespace=true})
        self.buffer = {}
    else
        -- convert ASCII numbers to strings and append to self.buffer
        table.insert(self.buffer, string.char(f))
    end
end

-- convert list to individual floats
function SsrClient:in_2_list(atoms)
    for _, f in ipairs(atoms) do self:in_2_float(f) end
end
