import * as THREE from "three";
import * as TWEEN from "@tweenjs/tween.js";
import { GLTFLoader } from "three/examples/jsm/loaders/GLTFLoader";
import { DRACOLoader } from "three/examples/jsm/loaders/DRACOLoader";
import { OrbitControls } from "three/examples/jsm/controls/OrbitControls";
import { useEffect, useRef, useState } from "react";

export default function MainScene(props: {
  toggleModal: boolean;
  setToggleModalCallback: any;
  pads: string[];
  setPadsCallback: any;
}) {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    // Resize the canvas when the window is resized
    window.addEventListener("resize", () => {
      renderer.setSize(window.innerWidth, window.innerHeight);
      camera.aspect = window.innerWidth / window.innerHeight;
      camera.updateProjectionMatrix();
    });

    // Check if the canvas is loaded
    if (canvasRef.current === null) {
      return;
    }

    /**
     * Initialize the scene, camera, light, and renderer
     */

    // Scene
    const scene = new THREE.Scene();

    // Camera
    const camera = new THREE.PerspectiveCamera(
      20,
      window.innerWidth / window.innerHeight,
      0.1,
      1000
    );

    // Light
    const ambientLight = new THREE.AmbientLight(0x404040, 10);
    scene.add(ambientLight);

    // Add a point light to the scene
    const pointLight = new THREE.PointLight(0xffffff, 5, 10);
    pointLight.position.set(10, 10, 10);
    scene.add(pointLight);

    // Renderer
    const renderer = new THREE.WebGLRenderer({
      antialias: true,
      canvas: canvasRef.current,
    });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(window.innerWidth, window.innerHeight);

    // Set the background color of the scene to white
    renderer.setClearColor(0xd8dee9);

    // Load our assets
    const dracoLoader = new DRACOLoader();
    const loader = new GLTFLoader();

    dracoLoader.setDecoderPath("/draco/");
    loader.setDRACOLoader(dracoLoader);

    let desiredChild: THREE.Object3D[] = [];
    let GLTFScene: any;

    loader.load(
      "/models/main.glb",
      (gltf) => {
        console.log("Loaded GLB file", gltf.scene);
        scene.add(gltf.scene);
        console.log(gltf.scene);
        GLTFScene = gltf.scene;

        for (let child of gltf.scene.children) {
          if (
            child.name === "PadLeft" ||
            child.name === "PadTop" ||
            child.name === "PadRight" ||
            child.name === "PadBottom"
          ) {
            desiredChild.push(child);
          }
        }
      },
      undefined,
      (error) => {
        console.error("An error occurred while loading the GLB file", error);
      }
    );

    // Instantiate a Raycaster and a Vector2 for the mouse position
    const raycaster = new THREE.Raycaster();
    const mouse = new THREE.Vector2();

    // Function to handle mouse clicks
    function onClick(event: MouseEvent) {
      event.preventDefault();

      // Calculate the mouse position in normalized device coordinates (-1 to +1)
      mouse.x = (event.clientX / window.innerWidth) * 2 - 1;
      mouse.y = -(event.clientY / window.innerHeight) * 2 + 1;

      // Update the picking ray with the camera and mouse position
      raycaster.setFromCamera(mouse, camera);

      // Calculate objects intersecting the picking ray
      const intersects = raycaster.intersectObjects(scene.children, true);

      for (let intersect of intersects) {
        if (desiredChild.includes(intersect.object)) {
          // console.log(intersect);

          desiredChild.forEach((child) => {
            if (child.name === intersect.object.name) {
              // Add the color to the object that was clicked
              if (child.material.color.getHex() === 0x000000) {
                child.material.color.set(2960685);
                props.setPadsCallback((oldPads: string[]) => {
                  return oldPads.filter((pad) => pad !== child.name);
                });
              } else {
                child.material.color.set(0x000000);
                props.setPadsCallback((oldPads: string[]) => {
                  return [...oldPads, child.name];
                });
              }
            }
          });
          moveCameraTo(new THREE.Vector3(5, 10, 0), 800);

          const newPosition = GLTFScene.position.clone();
          if (newPosition.z === 0) {
            newPosition.z += 2;
            moveObjectTo(GLTFScene, newPosition, 800);
          }

          props.setToggleModalCallback(true);
          break;
        }
      }
    }

    // Function to smoothly move the camera to a new position
    function moveCameraTo(newPosition: THREE.Vector3, duration: number) {
      const initialPosition = new THREE.Vector3().copy(camera.position);
      const tween = new TWEEN.Tween(initialPosition)
        .to(newPosition, duration)
        .easing(TWEEN.Easing.Quadratic.Out)
        .onUpdate(() => {
          camera.position.set(
            initialPosition.x,
            initialPosition.y,
            initialPosition.z
          );

          camera.lookAt(scene.position);
        })
        .start();
    }

    // Function to smoothly move an object to a new position
    function moveObjectTo(
      object: THREE.Object3D,
      newPosition: THREE.Vector3,
      duration: number
    ) {
      const initialPosition = new THREE.Vector3().copy(object.position);
      const tween = new TWEEN.Tween(initialPosition)
        .to(newPosition, duration)
        .easing(TWEEN.Easing.Quadratic.Out)
        .onUpdate(() => {
          object.position.set(
            initialPosition.x,
            initialPosition.y,
            initialPosition.z
          );
        })
        .start();
    }

    // Add event listener for mouse clicks
    window.addEventListener("click", onClick);

    // Camera Position
    camera.position.set(5, 5, 5);

    // Add OrbitControls to enable zoom functionality
    const controls = new OrbitControls(camera, canvasRef.current);
    controls.enableZoom = true;
    controls.enablePan = false;
    controls.enableDamping = true;
    controls.update();

    // Animate the cube
    function animate() {
      requestAnimationFrame(animate);

      TWEEN.update();

      controls.update();
      renderer.render(scene, camera);
    }
    animate();
  }, []);

  return (
    <div className="h-screen w-screen">
      <canvas ref={canvasRef} className="h-full w-full"></canvas>
    </div>
  );
}
