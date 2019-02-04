import { SceneViewer } from './sceneviewer.js';
import './style.css';

let HOST = 'localhost';
let PORT = '9422';

let viewport = document.createElement('div');
viewport.id = 'viewport';
document.body.appendChild(viewport);
let viewer = new SceneViewer(viewport);
viewer.animate();

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
