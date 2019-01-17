function varargout = gui_for_ssr(varargin)
% GUI_FOR_SSR MATLAB code for gui_for_ssr.fig
%      GUI_FOR_SSR, by itself, creates a new GUI_FOR_SSR or raises the existing
%      singleton*.
%
%      H = GUI_FOR_SSR returns the handle to a new GUI_FOR_SSR or the handle to
%      the existing singleton*.
%
%      GUI_FOR_SSR('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in GUI_FOR_SSR.M with the given input arguments.
%
%      GUI_FOR_SSR('Property','Value',...) creates a new GUI_FOR_SSR or raises the
%      existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before gui_for_ssr_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to gui_for_ssr_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help gui_for_ssr

% Last Modified by GUIDE v2.5 10-Jan-2019 11:37:49

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
                   'gui_Singleton',  gui_Singleton, ...
                   'gui_OpeningFcn', @gui_for_ssr_OpeningFcn, ...
                   'gui_OutputFcn',  @gui_for_ssr_OutputFcn, ...
                   'gui_LayoutFcn',  [] , ...
                   'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT


% --- Executes just before gui_for_ssr is made visible.
function gui_for_ssr_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to gui_for_ssr (see VARARGIN)

% Choose default command line output for gui_for_ssr
handles.output = hObject;

% Update handles structure
guidata(hObject, handles);

% UIWAIT makes gui_for_ssr wait for user response (see UIRESUME)
% uiwait(handles.figure1);

% ------------------- Case 1: You have tcpclient ------------------------ %
global tcp_obj;

disp( 'Attempting to connect to SSR.' );

% connect to SSR
tcp_obj = tcpclient( '127.0.0.1', 4711 );
% -------------------------- End case 1 --------------------------------- %

% % ---------------- Case 2: You don't have tcpclient ------------------- %
% global jtcp_obj;
% 
% disp( 'Attempting to connect to SSR.' );
% 
% % connect to SSR
% jtcp_obj = jtcp( 'request', '127.0.0.1', 4711, 'serialize', false );
% % ------------------------- End case 2 -------------------------------- %

% Create an audio signal
fs = 44100;
N = 8000;
noise = randn( N, 1 ) .* hann( N );
sig = [ zeros( N, 1 ); noise; zeros( N, 1 ); noise; zeros( N, 1 ); noise; zeros( N, 1 ) ];

% Or read a signal from a file
% [ sig, fs ] = audioread( 'anechoic_drums_48k.wav' );

% Find the audio device ID of JACK
devices = audiodevinfo;

device_id = 0;

% Find JACk by name
for n = 1 : size( devices.output, 2 )
    if contains( devices.output(n).Name, 'JackRouter' ) 
        % Get the ID
        device_id = devices.output(n).ID;
        break;
    end
end

if ~device_id
    error( 'Cannot find Motu interface.' );
end

fprintf( 'Using device ''%s''.\n', devices.output(n).Name );


global audio_player;
audio_player = audioplayer( sig, fs, 16, device_id );

% enable looping of the audio signal
dummy.stopPlayback = false;
audio_player.UserData = false;
audio_player.StopFcn = @audioplayerLoopingStopFcn;

play( audio_player );

pause( 1 );

no_of_stimuli_in_ssr = 2;

% disconnect system input
for n = 1 : no_of_stimuli_in_ssr
    unix( sprintf( '/usr/local/bin/jack_disconnect system:capture_1 BrsRenderer:in_%d', n ) );
end

% disconnect system out 
unix( '/usr/local/bin/jack_disconnect MATLAB:out1 system:playback_1' );
unix( '/usr/local/bin/jack_disconnect MATLAB:out2 system:playback_2' );

% connect audio to SSR
for n = 1 : no_of_stimuli_in_ssr
    unix( sprintf( '/usr/local/bin/jack_connect MATLAB:out1 BrsRenderer:in_%d', n ) );
end

% now we can pause the audio
audio_player.UserData = true;
pause( audio_player );
audio_player.UserData = false;

% handle close requests
set( hObject, 'CloseRequestFcn', {@close_gui, guidata(hObject)}); 

button_play_a_Callback(hObject, eventdata, handles);

set( handles.button_play, 'Enable', 'on' );
set( handles.button_stop, 'Enable', 'off' );

set( handles.button_play_a, 'Enable', 'off' );
set( handles.button_play_b, 'Enable', 'off' );



% --- Outputs from this function are returned to the command line.
function varargout = gui_for_ssr_OutputFcn(hObject, eventdata, handles) 
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes on button press in button_play_a.
function button_play_a_Callback(hObject, eventdata, handles)
% hObject    handle to button_play_a (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

set( handles.button_play_a, 'BackgroundColor', [ .2 .6 1 ] );
set( handles.button_play_b, 'BackgroundColor', 'default' );

% ------------------- Case 1: You have tcpclient ------------------------ %
global tcp_obj;

% Play source 1
write( tcp_obj, [ uint8( sprintf( '<request><source id="%d" mute="false"/></request>', 1 ) ) 0 ] );
% Mute source 2
write( tcp_obj, [ uint8( sprintf( '<request><source id="%d" mute="true"/></request>', 2 ) ) 0 ] );
% -------------------------- End case 1 --------------------------------- %

% % ---------------- Case 2: You don't have tcpclient ------------------- %
% global jtcp_obj;
% 
% % Play source 1
% jtcp( 'write', jtcp_obj, [ int8( sprintf( '<request><source id="%d" mute="false"/></request>', 1 ) ) 0 ] );
% % Mute source 2
% jtcp( 'write', jtcp_obj, [ int8( sprintf( '<request><source id="%d" mute="true"/></request>', 2 ) ) 0 ] );
% % ------------------------- End case 2 -------------------------------- %


% --- Executes on button press in button_play_b.
function button_play_b_Callback(hObject, eventdata, handles)
% hObject    handle to button_play_b (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

set( handles.button_play_a, 'BackgroundColor', 'default' );
set( handles.button_play_b, 'BackgroundColor', [ .2 .6 1 ] );

% ------------------- Case 1: You have tcpclient ------------------------ %
global tcp_obj;

% Play source 2
write( tcp_obj, [ uint8( sprintf( '<request><source id="%d" mute="false"/></request>', 2 ) ) 0 ] );
% Mute source 1
write( tcp_obj, [ uint8( sprintf( '<request><source id="%d" mute="true"/></request>', 1 ) ) 0 ] );
% -------------------------- End case 1 --------------------------------- %

% % ---------------- Case 2: You don't have tcpclient ------------------- %
% global jtcp_obj;
% 
% % Play source 2
% jtcp( 'write', jtcp_obj, [ int8( sprintf( '<request><source id="%d" mute="false"/></request>', 2 ) ) 0 ] );
% % Mute source 1
% jtcp( 'write', jtcp_obj, [ int8( sprintf( '<request><source id="%d" mute="true"/></request>', 1 ) ) 0 ] );
% % ------------------------- End case 2 -------------------------------- %


% --- Executes on button press in button_play.
function button_play_Callback(hObject, eventdata, handles)
% hObject    handle to button_play (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

set( handles.button_play, 'Enable', 'off' );
set( handles.button_stop, 'Enable', 'on' );

set( handles.button_play_a, 'Enable', 'on' );
set( handles.button_play_b, 'Enable', 'on' );

global audio_player;
resume( audio_player );

% --- Executes on button press in button_stop.
function button_stop_Callback(hObject, eventdata, handles)
% hObject    handle to button_stop (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

set( handles.button_play, 'Enable', 'on' );
set( handles.button_stop, 'Enable', 'off' );

set( handles.button_play_a, 'Enable', 'off' );
set( handles.button_play_b, 'Enable', 'off' );

global audio_player;
audio_player.UserData = true;
pause( audio_player );
audio_player.UserData = false;

% --- This is required for looping the audio.
function audioplayerLoopingStopFcn(haudioplayer, eventStruct)
if ( haudioplayer.UserData == false)
	play( haudioplayer );
end

function close_gui(hObject, eventdata, handles )

% Save user data
file_name = sprintf( 'results_%s.mat', datestr( clock, 'yyyy-mm-dd_HH-MM-SS' ) );

global user_response;
save( file_name, 'user_response' );

% Stop audio
global audio_player;
audio_player.UserData = true;
stop( audio_player );
audio_player.UserData = false;

fprintf( 'Closing. Saving results in %s.\n', file_name );

delete( gcf );


% --- Executes on slider movement.
function slider_response_Callback(hObject, eventdata, handles)
% hObject    handle to slider_response (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'Value') returns position of slider
%        get(hObject,'Min') and get(hObject,'Max') to determine range of slider

global user_response;
user_response = get( hObject, 'Value' );

disp( user_response );

% --- Executes during object creation, after setting all properties.
function slider_response_CreateFcn(hObject, eventdata, handles)
% hObject    handle to slider_response (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: slider controls usually have a light gray background.
if isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor',[.9 .9 .9]);
end

