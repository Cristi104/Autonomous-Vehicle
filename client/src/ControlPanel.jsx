import { useEffect, useRef, useState } from "react";
import ENDPOINT_URL from "./data.js";
import Slider from "./Slider.jsx";

export default function VideoStream() {
    const [data, setData] = useState({})
    const [loaded, setLoaded] = useState(false)
    const wsRef = useRef(null);
    const [isOpen, setIsOpen] = useState(false);

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
      if (isOpen) {
        return
      }
      async function fetch_data() {
        const response = await fetch(`http://${ENDPOINT_URL}/config`)
        const fetchdata = await response.json()
        setData(fetchdata)
        setLoaded(true);
      }
      fetch_data()
    }, [isOpen])
    
    if (!loaded) {
      
      return (
        <div className="bg-black/50 p-2 absolute bottom-0 left-0 flex rounded-md">
          <div className="text-sky-200">
            Loading
          </div>
        </div>
      );
    }

    return (
      <div className="bg-black/50 p-2 absolute bottom-0 left-0 flex rounded-md">
      <p>test</p>
        <div className="text-sky-200">
          <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
            wsRef.current.send('on: 0')
          }}>STOP</button>
          
          <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
            wsRef.current.send('on: 1')
          }}>GO</button>
          <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
            setIsOpen(true)
          }}>Options</button>

          {isOpen && (
            <div className="fixed inset-0 flex items-center justify-center bg-black/50" onClick={() => setIsOpen(false)}>
              <div className="rounded-2xl shadow-xl p-6 w-240 bg-slate-900" onClick={(e) => e.stopPropagation()}>
                <div>
                <p>General settings</p>
                <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
                  wsRef.current.send('debug_view: 0')
                }}>debug view off</button>
                
                <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
                  wsRef.current.send('debug_view: 1')
                }}>debug view on</button>
                <Slider ws={wsRef} name="speed" minv={0} maxv={100} step={1} defv={data["speed"]}/>
                <p>PID controller tweaks</p>
                <Slider ws={wsRef} name="kp" minv={0} maxv={2} step={0.01} defv={data["kp"]}/>
                <Slider ws={wsRef} name="ki" minv={0} maxv={0.5} step={0.001} defv={data["ki"]}/>
                <Slider ws={wsRef} name="kd" minv={0} maxv={0.5} step={0.001} defv={data["kd"]}/>
                <Slider ws={wsRef} name="kdef" minv={-0.5} maxv={0.5} step={0.001} defv={data["kdef"]}/>
                <p>Lane Detector</p>
                <p>Canny edge detection</p>
                <Slider ws={wsRef} name="LD_canny_min" minv={0} maxv={255} step={1} defv={data["LD_canny_min"]}/>
                <Slider ws={wsRef} name="LD_canny_max" minv={0} maxv={255} step={1} defv={data["LD_canny_max"]}/>
                <p>Resolution settings</p>
                <Slider ws={wsRef} name="LD_source_height" minv={240} maxv={1080} step={1} defv={data["LD_source_height"]}/>
                <Slider ws={wsRef} name="LD_source_width" minv={320} maxv={1920} step={1} defv={data["LD_source_width"]}/>
                <Slider ws={wsRef} name="source_height" minv={240} maxv={1080} step={1} defv={data["source_height"]}/>
                <Slider ws={wsRef} name="source_width" minv={320} maxv={1920} step={1} defv={data["source_width"]}/>
                </div>
                <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
                  setIsOpen(false)
                }}>Close</button>
                <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
                  wsRef.current.send('save: 1')
                }}>save</button>

              </div>
            </div>
          )}

        </div>
      </div>
    );
}
