import React, { useRef, useEffect, useState } from "react";
import { AiOutlineClose } from "react-icons/ai";
import WebSocket from "isomorphic-ws";

interface ModalProps {
  pads: string[];
  setToggleModalCallback: (value: boolean) => void;
}

const Modal: React.FC<ModalProps> = ({ pads, setToggleModalCallback }) => {
  const uniquePads = [...new Set(pads)];
  const [padsData, setPadsData] = useState<any[]>([]);
  const [def, setDef] = useState<any[]>([]);

  useEffect(() => {
    const ws = new WebSocket("ws://0.0.0.0:5000/ws");

    ws.onopen = () => {
      console.log("WebSocket connection established");
      ws.send("Hello, WebSocket!");
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
  }, []);

  return (
    <div className="absolute right-[4vw] h-fit min-h-[70%] w-6/12 bg-white p-8">
      <AiOutlineClose
        onClick={() => setToggleModalCallback(false)}
        className="absolute right-0 top-0 m-4"
      />
      {uniquePads.map((pad) => (
        <>
          <p>
            {pad} | Thresholds:{" "}
            {padsData[0] === "thresholds"
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
              : 0}
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
            className="w-full"
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
        </>
      ))}
    </div>
  );
};

export default Modal;
