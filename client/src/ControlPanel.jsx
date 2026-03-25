import { useEffect, useRef, useState } from "react";
import ENDPOINT_URL from "./data.js";

export default function VideoStream() {
    const [speed, setSpeed] = useState(50)
    const [kp, setKp] = useState(0.3)
    const [ki, setKi] = useState(0)
    const [kd, setKd] = useState(0)
    const [kdef, setKdef] = useState(0)
    const wsRef = useRef(null);

    useEffect(() => {
        wsRef.current = new WebSocket(`ws://${ENDPOINT_URL}/ws/control`);

        wsRef.current.onmessage = (event) => {
        };

        wsRef.current.onerror = (err) => {
            console.error("WebSocket error:", err);
        };

        return () => {
            wsRef.current?.close();
        };
    }, []);

    useEffect(() => {
      async function fetch_data() {
        const response = await fetch(`http://${ENDPOINT_URL}/config`)
        console.log(response)
        const data = await response.json()
        setSpeed(data["speed"])
        setKp(data["kp"])
        setKi(data["ki"])
        setKd(data["kd"])
        setKdef(data["kdef"])
        console.log(data)
      }
      fetch_data()
      // fetch(`http://${ENDPOINT_URL}:8000/config`)
      //   .then(response => {
      //     if (!response.ok) throw new Error("Network response was not ok");
      //     return response.json();
      //   })
      //   .then(data => console.log(data))
      //   .catch(err => console.error("Fetch error:", err));

    }, [])
    

    return (
        <div className="col-span-4 col-start-1 row-span-1 row-start-5 flex">
          <div className="text-sky-200">
            <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
              wsRef.current.send('[STOP]')
            }}>STOP</button>
            
            <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
              wsRef.current.send('[GO]')
            }}>GO</button>

            <div className="m-1 flex">
              <p className="mx-1 w-22">Speed: {speed}</p>
              <input className="w-60 my-auto h-2 bg-blue-800 rounded-md appearance-none cursor-pointer accent-sky-200" 
                type="range" min="0" max="100" value={speed} onChange={(e) => {setSpeed(Number(e.target.value))}} />
              <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
                wsRef.current.send(`[SPEED] ${speed}`)
              }}>Update</button>
            </div>

            <div className="m-1 flex">
              <p className="mx-1 w-22">kp: {kp}</p>
              <input className="w-60 my-auto h-2 bg-blue-800 rounded-md appearance-none cursor-pointer accent-sky-200" 
                type="range" min="0" max="2" step="0.01" value={kp} onChange={(e) => {setKp(Number(e.target.value))}} />
              <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
                wsRef.current.send(`[KP] ${kp}`)
              }}>Update</button>
            </div>

            <div className="m-1 flex">
              <p className="mx-1 w-22">ki: {ki}</p>
              <input className="w-60 my-auto h-2 bg-blue-800 rounded-md appearance-none cursor-pointer accent-sky-200" 
                type="range" min="0" max="2" step="0.01" value={ki} onChange={(e) => {setKi(Number(e.target.value))}} />
              <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
                wsRef.current.send(`[KI] ${ki}`)
              }}>Update</button>
            </div>

            <div className="m-1 flex">
              <p className="mx-1 w-22">kd: {kd}</p>
              <input className="w-60 my-auto h-2 bg-blue-800 rounded-md appearance-none cursor-pointer accent-sky-200" 
                type="range" min="0" max="2" step="0.01" value={kd} onChange={(e) => {setKd(Number(e.target.value))}} />
              <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
                wsRef.current.send(`[KD] ${kd}`)
              }}>Update</button>
            </div>
            
            <div className="m-1 flex">
              <p className="mx-1 w-22">kdef: {kdef}</p>
              <input className="w-60 my-auto h-2 bg-blue-800 rounded-md appearance-none cursor-pointer accent-sky-200" 
                type="range" min="-2" max="2" step="0.01" value={kdef} onChange={(e) => {setKdef(Number(e.target.value))}} />
              <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
                wsRef.current.send(`[KDEF] ${kdef}`)
              }}>Update</button>
            </div>
          </div>
        </div>
    );
}
