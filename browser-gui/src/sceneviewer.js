import * as THREE from 'three';
// See https://stackoverflow.com/a/44960831/
import 'three-examples/controls/OrbitControls';
import 'three-examples/controls/TransformControls';


function directionalLight() {
  var color = 0xffffff;
  var intensity = 1;

  var light = new THREE.DirectionalLight(color, intensity);
  light.name = 'MyDirectionalLight';
  light.target.name = 'MyDirectionalLight Target';

  light.position.set(1, 1, 1);
  return light;
}


function pointLight() {
  var color = 0xffffff;
  var intensity = 1;
  var distance = 0;

  var light = new THREE.PointLight(color, intensity, distance);
  light.name = 'MyPointLight';
  light.position.x = 10;
  light.position.y = 10;
  light.position.z = 10;
  light.scale.set(.1, .1, .1);
  return light;
}


function hemisphereLight() {
  var skyColor = 0x70f0ff;
  var groundColor = 0x906000;
  var intensity = 1;

  var light = new THREE.HemisphereLight(skyColor, groundColor, intensity);
  light.name = 'MyHemisphereLight';
  light.position.x = 0;
  light.position.y = 0;
  light.position.z = 10;
  light.scale.set(.1, .1, .1);
  return light;
}


function kiteGeometry() {
  // object is pointing in positive x direction
  // positive z is top!
  var kite = new THREE.Geometry();
  kite.vertices.push(new THREE.Vector3(.8, 0, 0));  // 0: front
  kite.vertices.push(new THREE.Vector3(0, .4, 0));  // 1: left
  kite.vertices.push(new THREE.Vector3(-.2, 0, 0));  // 2: back
  kite.vertices.push(new THREE.Vector3(0, -.4, 0));  // 3: right
  kite.vertices.push(new THREE.Vector3(0, 0, .2));  // 4: top
  kite.vertices.push(new THREE.Vector3(0, 0, -.1));  // 5: bottom

  kite.faces.push(new THREE.Face3(0, 1, 4));
  kite.faces.push(new THREE.Face3(1, 2, 4));
  kite.faces.push(new THREE.Face3(2, 3, 4));
  kite.faces.push(new THREE.Face3(3, 0, 4));

  kite.faces.push(new THREE.Face3(1, 0, 5));
  kite.faces.push(new THREE.Face3(2, 1, 5));
  kite.faces.push(new THREE.Face3(3, 2, 5));
  kite.faces.push(new THREE.Face3(0, 3, 5));

  kite.computeFaceNormals();

  // object is pointing in positive y direction
  kite.rotateZ(Math.PI / 2);

  return kite;
}


function create_proxy(object) {
  object.temp_position = new THREE.Vector3();
  object.fake_position = new Proxy(object.temp_position, {
    get(target, name, receiver) {
      switch (name) {
        case 'copy':
          return function (other) {
            target.copy(other);
            return receiver;
          };
        case 'add':
          return function (other) {
            target.add(other);
            return receiver;
          };
        default:
          return Reflect.get(object.position, name, receiver);
      }
    }
  });

  object.temp_quaternion = new THREE.Quaternion();
  object.fake_quaternion = new Proxy(object.temp_quaternion, {
    get(target, name, receiver) {
      switch (name) {
        case 'copy':
          return function (other) {
            target.copy(other);
            return receiver;
          };
        case 'multiply':
          return function (other) {
            target.multiply(other);
            return receiver;
          };
        case 'normalize':
          return function () {
            target.normalize();
            return receiver;
          }
        default:
          return Reflect.get(object.quaternion, name, receiver);
      }
    }
  });

  object.proxy = new Proxy(object, {
    get(target, name, receiver) {
      switch (name) {
        case 'position':
          return object.fake_position;
        case 'quaternion':
          return object.fake_quaternion;
        default:
          return Reflect.get(target, name, receiver);
      }
    }
  });
}


export class SceneViewer {
  constructor(dom) {
    this.send = undefined;  // This has to be set from outside
    this.animate = this.animate.bind(this);
    this.onWindowResize = this.onWindowResize.bind(this);
    this.onMouseUp = this.onMouseUp.bind(this);
    this.onTouchEnd = this.onTouchEnd.bind(this);
    this.getMousePosition = this.getMousePosition.bind(this);
    this.switch_to_3d = this.switch_to_3d.bind(this);
    this.switch_to_top = this.switch_to_top.bind(this);
    this.switch_to_ego = this.switch_to_ego.bind(this);
    this.switch_to_translation = this.switch_to_translation.bind(this);
    this.switch_to_rotation = this.switch_to_rotation.bind(this);
    this.dom = dom;

    this.camera_3d = new THREE.PerspectiveCamera();
    this.camera_3d.fov = 50;
    this.camera_3d.near = 0.01;
    this.camera_3d.far = 1000;
    // TODO: store camera position in browser (localStorage)?
    this.camera_3d.position.set(0, -10, 5);
    this.camera_3d.up.set(0, 0, 1);

    this.camera_ego = new THREE.PerspectiveCamera();
    this.camera_ego.fov = 80;  // Slight fisheye effect
    this.camera_ego.near = 0.01;
    this.camera_ego.far = 1000;
    this.camera_ego.lookAt(0, 1, 0);

    this.frustum_height = 10;  // meters
    this.camera_top = new THREE.OrthographicCamera();
    this.camera_top.top = this.frustum_height / 2;
    this.camera_top.bottom = -this.frustum_height / 2;
    this.camera_top.near = -100000;
    this.camera_top.far = 100000;
    this.camera_top.zoom = 1;

    this.scene = new THREE.Scene();
    this.scene.background = new THREE.Color(0xaaaaaa);
    this.scene.fog = null;

    let grid = new THREE.GridHelper(20, 20, 0x444444, 0x888888);
    grid.rotation.x = Math.PI/2;
    var array = grid.geometry.attributes.color.array;
    for (var i = 0; i < array.length; i += 60) {
      for (var j = 0; j < 12; j++) {
        array[i + j] = 0.26;
      }
    }
    this.scene.add(grid);

    this.renderer = new THREE.WebGLRenderer({ antialias: true });
    this.renderer.setPixelRatio(window.devicePixelRatio);
    if (this.renderer.shadowMap) this.renderer.shadowMap.enabled = true;
    this.dom.appendChild(this.renderer.domElement);

    window.addEventListener('resize', this.onWindowResize);
    this.onWindowResize();

    // https://threejs.org/docs/#examples/controls/OrbitControls
    this.controls_3d = new THREE.OrbitControls(this.camera_3d, this.dom);
    //this.controls_3d.minDistance = 0;
    //this.controls_3d.maxDistance = 1500;  // default: infinite
    this.controls_3d.enableDamping = true;
    this.controls_3d.enableKeys = true;  // only for panning (arrow keys)
    this.controls_3d.dampingFactor = 0.2;
    this.controls_3d.screenSpacePanning = true;
    this.controls_3d.panSpeed = 0.3;
    this.controls_3d.rotateSpeed = 0.1;

    //this.scene.add(pointLight());
    this.scene.add(directionalLight());
    //this.scene.add(hemisphereLight());
    this.scene.add(new THREE.AmbientLight(0x505050));

    this.sources = {};
    // TODO: binaural vs. loudspeakers?
    this.createReference();
    this.reference_offset = new THREE.Mesh(
      kiteGeometry(), new THREE.MeshBasicMaterial({ wireframe: true }));
    this.reference.add(this.reference_offset);
    this.reference_offset.add(this.camera_ego);

    // https://threejs.org/docs/#examples/controls/TransformControls
    this.transformControls = new THREE.TransformControls(this.camera_3d, this.dom);
    this.transformControls.setSize(0.7);

    this.scene.add(this.transformControls);

    this.raycaster = new THREE.Raycaster();
    this.onDownPosition = new THREE.Vector2();
    this.onUpPosition = new THREE.Vector2();

    let that = this;

    this.transformControls.addEventListener('dragging-changed', function (event) {
      that.controls_3d.enabled = !event.value;
    });

    this.transformControls.addEventListener('objectChange', function (event) {
      if (event.target.object === that.reference.proxy) {
        switch (event.target.mode) {
          case 'translate':
            let p = that.reference.temp_position;
            that.send(["state", {"ref-pos": [p.x, p.y, p.z]}]);
            break;
          case 'rotate':
            let q = that.reference.temp_quaternion;
            that.send(["state", {"ref-rot": [q.x, q.y, q.z, q.w]}]);
            break;
          default:
            console.log('Invalid mode: ' + event.target.mode);
        }
      } else {
        let source = event.target.object;
        switch (event.target.mode) {
          case 'translate':
            let p = source.temp_position;
            that.send(["mod-src", {[source.ssr_id]: {"pos": [p.x, p.y, p.z]}}]);
            break;
          case 'rotate':
            let q = source.temp_quaternion;
            that.send(["mod-src", {[source.ssr_id]: {"rot": [q.x, q.y, q.z, q.w]}}]);
            break;
          default:
            console.log('Invalid mode: ' + event.target.mode);
        }
      }
    });

    this.dom.addEventListener('mousedown', function (event) {
      let array = that.getMousePosition(event.clientX, event.clientY);
      that.onDownPosition.fromArray(array);
      document.addEventListener('mouseup', that.onMouseUp, false );
    }, false);

    this.dom.addEventListener('touchstart', function (event) {
      let touch = event.changedTouches[0];
      let array = that.getMousePosition(touch.clientX, touch.clientY);
      that.onDownPosition.fromArray(array);
      document.addEventListener('touchend', that.onTouchEnd, false );
    }, false);

    this.switch_to_3d();
    this.switch_to_translation();
  }

  switch_to_3d() {
    this.camera = this.camera_3d;
    if (this.selected) {
      this.transformControls.attach(this.selected);
    }
    this.controls_3d.enabled = true;
  }

  switch_to_top() {
    // TODO: hide control-buttons?
    this.camera = this.camera_top;
    this.controls_3d.enabled = false;
    this.transformControls.detach();
  }

  switch_to_ego() {
    // TODO: hide control-buttons?
    this.camera = this.camera_ego;
    this.controls_3d.enabled = false;
    this.transformControls.detach();
  }

  switch_to_translation() {
    this.transformControls.setMode('translate');
    this.transformControls.setSpace('global');
  }

  switch_to_rotation() {
    this.transformControls.setMode('rotate');
    this.transformControls.setSpace('local');
  }

  createReference() {
    let reference = new THREE.Group();
    let material = new THREE.MeshPhongMaterial({ color: 0x2685AA, precision: 'mediump' });
    let mesh = new THREE.Mesh(kiteGeometry(), material);
    mesh.scale.set(0.6, 0.6, 0.6);
    reference.add(mesh);

    create_proxy(reference);
    this.scene.add(reference);
    this.reference = reference;
  }

  createSource(source_id) {
    //let kiteMaterial = new THREE.MeshBasicMaterial({ color: 0x2685AA });
    let kiteMaterial = new THREE.MeshPhongMaterial({ color: 0xBC7349, precision: 'mediump' });
    //let kiteMaterial = new THREE.MeshLambertMaterial({ color: 0xBC7349 });

    let mesh = new THREE.Mesh(kiteGeometry(), kiteMaterial);
    this.scene.add(mesh);
    create_proxy(mesh);
    mesh.ssr_id = source_id;
    this.sources[source_id] = mesh;
  }

  updateSource(source_id, data) {
    // TODO: error if not available?
    let source = this.sources[source_id];
    let pos = data.pos;
    // TODO: check if Array?
    if (pos) {
      source.position.x = pos[0];
      source.position.y = pos[1];
      if (pos[2]) {
        source.position.z = pos[2];
      } else {
        source.position.z = 0;
      }
    }
    let rot = data.rot;
    // TODO: check if length == 4
    if (rot) {
      source.quaternion.set(...rot);
    }
    let active = data.active;
    if (active !== undefined) {
      source.visible = active;
    }
  }

  removeSource(source_id) {
    // TODO: check if source exists?
    var source = this.sources[source_id];
    if (source === undefined) {
      throw Error(`Source "${source_id}" doesn't exist`);
    }
    this.scene.remove(source);
    delete this.sources[source_id];
  }

  createLoudspeakers(loudspeakers) {
    // TODO: check if loudspeakers is array
    // TODO: make sure no loudspeakers exist yet

    let material = new THREE.MeshPhongMaterial({ color: 0x2685AA, precision: 'mediump' });
    for (let ls of loudspeakers) {
      // TODO: check if ls is object?

      let mesh = new THREE.Mesh(kiteGeometry(), material);
      mesh.scale.set(0.3, 0.3, 0.3);
      mesh.position.set(...ls.pos);
      mesh.quaternion.set(...ls.rot);
      this.reference.add(mesh);
    }
  }

  onWindowResize(event) {
    let aspect = this.dom.offsetWidth / this.dom.offsetHeight;
    this.camera_3d.aspect = aspect;
    this.camera_3d.updateProjectionMatrix();
    this.camera_ego.aspect = aspect;
    this.camera_ego.updateProjectionMatrix();
    this.camera_top.left = -this.frustum_height * aspect / 2;
    this.camera_top.right = this.frustum_height * aspect / 2;
    this.camera_top.updateProjectionMatrix();
    this.renderer.setSize(this.dom.offsetWidth, this.dom.offsetHeight);
  }

  getMousePosition(x, y) {
    let rect = this.dom.getBoundingClientRect();
    return [(x - rect.left) / rect.width, (y - rect.top) / rect.height];
  }

	onMouseUp(event) {
		let array = this.getMousePosition(event.clientX, event.clientY);
		this.onUpPosition.fromArray(array);
		this.handleClick();
		document.removeEventListener('mouseup', this.onMouseUp, false);
	}

	onTouchEnd(event) {
		let touch = event.changedTouches[0];
		let array = this.getMousePosition(touch.clientX, touch.clientY);
		this.onUpPosition.fromArray(array);
		this.handleClick();
		document.removeEventListener('touchend', this.onTouchEnd, false);
	}

	getIntersects(point) {
		this.raycaster.setFromCamera(
      new THREE.Vector2((point.x * 2) - 1, -(point.y * 2) + 1), this.camera);
    // TODO: Keep objects array around for re-use?
    let objects = Object.values(this.sources);
    let reference_mesh = this.reference.children[0];
    objects.push(reference_mesh);
		let intersects = this.raycaster.intersectObjects(objects);
    if (intersects.length === 0) {
      return null;
    }
    let object = intersects[0].object;
    if (object === reference_mesh) {
      return this.reference.proxy;
    }
    return object.proxy;
	}

	handleClick() {
		if (this.onDownPosition.distanceTo(this.onUpPosition) === 0) {
			let object = this.getIntersects(this.onUpPosition);
      this.select(object);
		}
	}

  select(object) {
		if (this.selected === object) return;
		this.selected = object;
		this.transformControls.detach();
		if (object !== null) {
      if (this.camera === this.camera_3d) {
        this.transformControls.attach(this.selected);
      }
		}
  }

  animate() {
    requestAnimationFrame(this.animate);

    //this.scene.updateMatrixWorld();

    if (this.controls_3d.enabled) {
      this.controls_3d.update();  // needed for enableDamping
    }
    this.renderer.render(this.scene, this.camera);
  }

  handle_message(msg) {
    msg = JSON.parse(msg);
    if (!(msg instanceof Array)) {
      throw Error(`Invalid message: ${msg}`);
    }
    while (true) {
      let command = msg.shift();
      if (command === undefined) {
        break;
      }

      // TODO: check if command is valid?

      let data = msg.shift();
      if (data === undefined) {
        throw Error(`No data for "${command}" command`);
      }

      switch (command) {
        case 'state':
          for (let attr in data) {
            let value = data[attr];
            switch (attr) {
              case 'loudspeakers':
                this.createLoudspeakers(value);
                break;
              case 'ref-pos':
                this.reference.position.set(...value);
                break;
              case 'ref-pos-offset':
                this.reference_offset.position.set(...value);
                break;
              case 'ref-rot':
                this.reference.quaternion.set(...value);
                break;
              case 'ref-rot-offset':
                this.reference_offset.quaternion.set(...value);
                break;
              default:
                // TODO: implement the rest!
                console.log(attr + ': ' + data);
            }
          }
          break;
        case 'new-src':
          // TODO: error if no keys available?
          for (let source_id in data) {
            if (this.sources.hasOwnProperty(source_id)) {
              throw Error(`Source "${source_id}" already exists`);
            }
            this.createSource(source_id);
            console.log(`Created source "${source_id}"`);
            this.updateSource(source_id, data[source_id]);
          }
          break;
        case 'mod-src':
          // TODO: error if no keys available?
          for (let source_id in data) {
            if (!this.sources.hasOwnProperty(source_id)) {
              throw Error(`Source "${source_id}" does not exist`);
            }
            this.updateSource(source_id, data[source_id]);
          }
          break;
        case 'del-src':
          // TODO: check if "data" is Array?
          data.forEach(item => this.removeSource(item));
          break;
        default:
          throw Error(`Unknown command: "${command}"`);
      }
    }
  }
}
