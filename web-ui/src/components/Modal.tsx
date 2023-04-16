import { AiOutlineClose } from "react-icons/ai";

export default function Modal(props: {
  pads: string[];
  setToggleModalCallback: any;
}) {
  let uniquePads = [...new Set(props.pads)];

  return (
    <div className="absolute right-[4vw] h-fit min-h-[70%] w-6/12 bg-white">
      <AiOutlineClose
        onClick={() => props.setToggleModalCallback(false)}
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
}
