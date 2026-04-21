import { useEffect, useRef, useState } from "react";

export default function Slider({ ws, name, minv, maxv, step, defv}) {
    const [value, setValue] = useState(defv);
    return (
      <div className="m-1 flex">
        <p className="mx-1 w-fit">{name}: {value}</p>
        <input className="w-60 ml-auto mr-0 my-auto h-2 bg-blue-800 rounded-md appearance-none cursor-pointer accent-sky-200" 
          type="range" min={minv} max={maxv} step={step} value={value} onChange={(e) => {setValue(Number(e.target.value))}} />
        <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
          ws.current.send(`${name}: ${value}`)
        }}>Update</button>
      </div>
    );
}
