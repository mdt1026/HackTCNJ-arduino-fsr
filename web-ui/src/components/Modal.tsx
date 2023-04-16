import React, { useState, useRef, useEffect, useCallback } from "react";
import { AiOutlineClose } from "react-icons/ai";

interface ModalProps {
  pads: string[];
  setToggleModalCallback: (value: boolean) => void;
  defaults: {
    thresholds: number[];
  } | null;
  onCloseWs: () => void;
}

// Maximum number of historical sensor values to retain
const MAX_SIZE = 1000;

const Modal: React.FC<ModalProps> = ({
  pads,
  setToggleModalCallback,
  defaults,
  onCloseWs,
}) => {
  let uniquePads = [...new Set(pads)];

  const [isWsReady, setIsWsReady] = useState(false);
  const webUIDataRef = useRef<{
    curValues: number[][];
    oldest: number;
    curThresholds: number[];
  }>({
    curValues: [],
    oldest: 0,
    curThresholds: [],
  });

  const wsRef = useRef<WebSocket | null>(null);
  const wsCallbacksRef = useRef<{ [key: string]: (msg: any) => void }>({});

  const emit = useCallback(
    (msg: object) => {
      if (!wsRef.current || !isWsReady) {
        throw new Error("emit() called when isWsReady !== true.");
      }
      wsRef.current.send(JSON.stringify(msg));
    },
    [isWsReady, wsRef]
  );

  wsCallbacksRef.current.values = (msg: { values: number[] }) => {
    const webUIData = webUIDataRef.current;
    if (webUIData.curValues.length < MAX_SIZE) {
      webUIData.curValues.push(msg.values);
    } else {
      webUIData.curValues[webUIData.oldest] = msg.values;
      webUIData.oldest = (webUIData.oldest + 1) % MAX_SIZE;
    }
  };

  wsCallbacksRef.current.thresholds = (msg: { thresholds: number[] }) => {
    webUIDataRef.current.curThresholds.length = 0;
    webUIDataRef.current.curThresholds.push(...msg.thresholds);
  };

  useEffect(() => {
    let cleaningUp = false;
    const webUIData = webUIDataRef.current;

    if (!defaults) {
      return;
    }

    webUIData.curValues.length = 0;
    webUIData.curValues.push(new Array(defaults.thresholds.length).fill(0));
    webUIData.oldest = 0;
    webUIDataRef.current.curThresholds.length = 0;
    webUIDataRef.current.curThresholds.push(...defaults.thresholds);

    const ws = new WebSocket("ws://" + window.location.host + "/ws");
    wsRef.current = ws;

    ws.addEventListener("open", (ev) => {
      setIsWsReady(true);
    });

    ws.addEventListener("error", (ev) => {
      ws.close();
    });

    ws.addEventListener("close", (ev) => {
      if (!cleaningUp) {
        onCloseWs();
      }
    });

    ws.addEventListener("message", (ev) => {
      const data = JSON.parse(ev.data);
      const action = data[0];
      const msg = data[1];

      if (wsCallbacksRef.current[action]) {
        wsCallbacksRef.current[action](msg);
      }
    });

    return () => {
      cleaningUp = true;
      setIsWsReady(false);
      ws.close();
    };
  }, [defaults, onCloseWs]);

  return (
    <div className="absolute right-[4vw] h-fit min-h-[70%] w-6/12 bg-white">
      <AiOutlineClose
        onClick={() => setToggleModalCallback(false)}
        className="absolute right-0 top-0 m-4"
      />
      {uniquePads.map((pad) => (
        <>
          <p>{pad}</p>
          <input type="range" min="1" max="1023" />
        </>
      ))}
    </div>
  );
};

export default Modal;
