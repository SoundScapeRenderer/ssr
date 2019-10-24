import { SceneViewer } from './sceneviewer.js';
import './style.css';

let HOST = 'localhost';
let PORT = '9422';

let viewport = document.createElement('div');
viewport.id = 'viewport';
document.body.appendChild(viewport);
let viewer = new SceneViewer(viewport);
viewer.animate();

let buttons = document.createElement('div');
buttons.id = 'buttons';
let button1 = document.createElement('button');
button1.textContent = '3D';
button1.onclick = viewer.switch_to_3d;
buttons.appendChild( button1 );
let button2 = document.createElement('button');
button2.textContent = 'top';
button2.onclick = viewer.switch_to_top;
buttons.appendChild( button2 );
let button3 = document.createElement('button');
button3.textContent = 'ego';
button3.onclick = viewer.switch_to_ego;
buttons.appendChild( button3 );
document.body.appendChild(buttons);

let socket = new WebSocket('ws://' + HOST + ':' + PORT, 'ssr-json');

socket.onopen = function()
{
  console.log('WebSocket connected to ' + socket.url + ', subprotocol: ' + socket.protocol);
  socket.send(JSON.stringify(["subscribe", ["scene", "renderer"]]));
};

socket.onmessage = function(msg)
{
  viewer.handle_message(msg.data);
};

socket.onerror = function()
{
  alert('WebSocket Error!');
};

socket.onclose = function(msg)
{
  console.log('WebSocket closed');
};
