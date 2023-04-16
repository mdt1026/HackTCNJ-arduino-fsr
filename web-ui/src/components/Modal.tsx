import React, { useRef, useEffect, useState } from "react";
import { AiOutlineClose } from "react-icons/ai";
import WebSocket from "isomorphic-ws";

interface ModalProps {
  pads: string[];
  setToggleModalCallback: (value: boolean) => void;
}

const ws = new WebSocket("ws://0.0.0.0:5000/ws");

const Modal: React.FC<ModalProps> = ({ pads, setToggleModalCallback }) => {
  const uniquePads = [...new Set(pads)];
  const [padsData, setPadsData] = useState<any[]>([]);
  const [def, setDef] = useState<any[]>([]);
  const [thresholds, setThresholds] = useState<any[]>([0, 0, 0, 0]);
  const [index, setIndex] = useState<number>(0);
  const modalRef = useRef<HTMLDivElement>(null);

  useEffect(() => {
    ws.onopen = () => {
      console.log("WebSocket connection established");
    };

    ws.onmessage = (event: MessageEvent) => {
      if (eval(event.data)[0] === "thresholds") {
        console.log(eval(event.data)[1]);
        setDef(eval(event.data));
      }
      setPadsData(eval(event.data));
      console.log("Received message:", event.data);
    };

    ws.onclose = () => {
      console.log("WebSocket connection closed");
    };

    const handleMouseMove = (event: MouseEvent) => {
      if (modalRef.current) {
        const centerX = modalRef.current.offsetWidth / 2;
        const centerY = modalRef.current.offsetHeight / 2;
        const mouseX = event.clientX - modalRef.current.offsetLeft - centerX;
        const mouseY = event.clientY - modalRef.current.offsetTop - centerY;
        const tiltX = -(mouseY / centerY) * 10;
        const tiltY = (mouseX / centerX) * 10;
        modalRef.current.style.transform = `perspective(1000px) rotateX(${tiltX}deg) rotateY(${tiltY}deg)`;
      }
    };

    window.addEventListener("mousemove", handleMouseMove);

    return () => {
      window.removeEventListener("mousemove", handleMouseMove);
    };
  }, []);

  return (
    <div
      ref={modalRef}
      className="absolute right-[4vw] h-fit min-h-[70%] w-6/12 transform bg-white p-8 transition-transform duration-500"
      style={{
        transformStyle: "preserve-3d",
        boxShadow: "0px 0px 20px #333333",
      }}
    >
      <AiOutlineClose
        onClick={() => setToggleModalCallback(false)}
        className="absolute right-0 top-0 m-4"
      />
      <div className="space-y-8">
        {uniquePads.map((pad) => (
          <div className="rounded-md border-0 p-4 shadow-xl">
            <h1 className="font-bold">{pad}</h1>
            <p>
              Thresholds:{" "}
              <input
                id={pad}
                type="number"
                value={
                  padsData[0] === "thresholds"
                    ? pad === "PadLeft"
                      ? padsData[1].thresholds[0]
                      : pad === "PadBottom"
                      ? padsData[1].thresholds[1]
                      : pad === "PadTop"
                      ? padsData[1].thresholds[2]
                      : pad === "PadRight"
                      ? padsData[1].thresholds[3]
                      : 0
                    : def.length !== 0
                    ? pad === "PadLeft"
                      ? def[1].thresholds[0]
                      : pad === "PadBottom"
                      ? def[1].thresholds[1]
                      : pad === "PadTop"
                      ? def[1].thresholds[2]
                      : pad === "PadRight"
                      ? def[1].thresholds[3]
                      : 0
                    : 0
                }
                onChange={(e) => {
                  console.log(e.target.id);
                  let i =
                    e.target.id === "PadLeft"
                      ? 0
                      : pad === "PadBottom"
                      ? 1
                      : pad === "PadTop"
                      ? 2
                      : 3;
                  let temp: any[] = thresholds;
                  temp[i] = Number(e.target.value);
                  setThresholds(temp);
                  console.log(JSON.stringify(["update_threshold", temp, i]));
                  ws.send(JSON.stringify(["update_threshold", temp, i])); // left bot top right
                }}
              />
            </p>
            <label htmlFor="padData">
              Force:{" "}
              {padsData[0] === "values"
                ? pad === "PadLeft"
                  ? padsData[1].values[0]
                  : pad === "PadBottom"
                  ? padsData[1].values[1]
                  : pad === "PadTop"
                  ? padsData[1].values[2]
                  : pad === "PadRight"
                  ? padsData[1].values[3]
                  : 0
                : 0}
            </label>
            <input
              className="h-8 w-full rounded-md border-0 outline-none"
              name="padData"
              type="range"
              min="1"
              max="1023"
              value={
                padsData[0] === "values"
                  ? pad === "PadLeft"
                    ? padsData[1].values[0]
                    : pad === "PadBottom"
                    ? padsData[1].values[1]
                    : pad === "PadTop"
                    ? padsData[1].values[2]
                    : pad === "PadRight"
                    ? padsData[1].values[3]
                    : 0
                  : 0
              }
            />
          </div>
        ))}
      </div>
    </div>
  );
};

export default Modal;
