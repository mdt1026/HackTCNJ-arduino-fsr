import * as THREE from "three";
import { useEffect, useRef } from "react";

export default function MainScene() {
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

    // Initialize the scene, camera, and renderer
    const scene = new THREE.Scene();
    const camera = new THREE.PerspectiveCamera(
      75,
      window.innerWidth / window.innerHeight,
      0.1,
      1000
    );
    const renderer = new THREE.WebGLRenderer({ canvas: canvasRef.current });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(window.innerWidth, window.innerHeight);

    // Add a cube to the scene
    const geometry = new THREE.BoxGeometry(1, 1, 1);
    const material = new THREE.MeshBasicMaterial({ color: 0x00ff00 });
    const cube = new THREE.Mesh(geometry, material);
    scene.add(cube);

    // Camera Position
    camera.position.z = 5;

    // Animate the cube
    function animate() {
      requestAnimationFrame(animate);

      cube.rotation.x += 0.01;
      cube.rotation.y += 0.01;

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
