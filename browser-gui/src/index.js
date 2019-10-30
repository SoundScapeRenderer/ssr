import { SceneViewer } from './sceneviewer.js';
import './style.css';

let HOST = 'localhost';
let PORT = '9422';

let viewport = document.createElement('div');
viewport.id = 'viewport';
document.body.appendChild(viewport);
let viewer = new SceneViewer(viewport);
viewer.animate();

let view_buttons = document.createElement('div');
view_buttons.id = 'view-buttons';
let button_3d = document.createElement('button');
button_3d.textContent = '3D';
button_3d.onclick = viewer.switch_to_3d;
view_buttons.appendChild(button_3d);
let button_top = document.createElement('button');
button_top.textContent = 'top';
button_top.onclick = viewer.switch_to_top;
view_buttons.appendChild(button_top);
let button_ego = document.createElement('button');
button_ego.textContent = 'ego';
button_ego.onclick = viewer.switch_to_ego;
view_buttons.appendChild(button_ego);
document.body.appendChild(view_buttons);

let control_buttons = document.createElement('div');
control_buttons.id = 'control-buttons';
let button_translate = document.createElement('button');
button_translate.textContent = 'move';
button_translate.onclick = viewer.switch_to_translation;
control_buttons.appendChild(button_translate);
let button_rotate = document.createElement('button');
button_rotate.textContent = 'rotate';
button_rotate.onclick = viewer.switch_to_rotation;
control_buttons.appendChild(button_rotate);
document.body.appendChild(control_buttons);

let socket;

fetch('config.json')
  .then(response => {
    if (response.ok) {
      return response.json();
    } else {
      return {
        "host": HOST,
        "port": PORT,
      }
    }}, error => {
      // This happens when index.html file was opened via file://
      return {
        "host": HOST,
        "port": PORT,
      }
    })
  .then(config => {
    socket = new WebSocket('ws://' + config.host + ':' + config.port, 'ssr-json');

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

    viewer.send = function(data)
    {
      socket.send(JSON.stringify(data));
    }
  }, error => {
    console.error(error);
    alert('Error loading config.json: ' + error);
  });
