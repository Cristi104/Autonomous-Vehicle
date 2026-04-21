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
      async function fetch_data() {
        const response = await fetch(`http://${ENDPOINT_URL}/config`)
        const fetchdata = await response.json()
        setData(fetchdata)
        setLoaded(true);
      }
      fetch_data()
    }, [])
    
    if (!loaded) {
      
      return (
        <div className="col-span-4 col-start-1 row-span-1 row-start-5 flex">
          <div className="text-sky-200">
            Loading
          </div>
        </div>
      );
    }

    return (
      <div className="col-span-4 col-start-1 row-span-1 row-start-5 flex">
        <div className="text-sky-200">
          <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
            wsRef.current.send('ON: 0')
          }}>STOP</button>
          
          <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
            wsRef.current.send('ON: 1')
          }}>GO</button>
          <button className="bg-blue-800 rounded-md mx-1 px-1 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
            setIsOpen(true)
          }}>Options</button>

          {isOpen && (
            <div className="fixed inset-0 flex items-center justify-center bg-black/50" onClick={() => setIsOpen(false)}>
              <div className="rounded-2xl shadow-xl p-6 w-240 bg-slate-900" onClick={(e) => e.stopPropagation()}>
                <div>
                <p>General settings</p>
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
                <p>Hough lines algoritm</p>
                <Slider ws={wsRef} name="LD_hough_max_line_gap" minv={0} maxv={255} step={1} defv={data["LD_hough_max_line_gap"]}/>
                <Slider ws={wsRef} name="LD_hough_min_line_length" minv={0} maxv={255} step={1} defv={data["LD_hough_min_line_length"]}/>
                <Slider ws={wsRef} name="LD_hough_theta" minv={0} maxv={Math.PI/45} step={Math.PI/1800} defv={data["LD_hough_theta"]}/>
                <Slider ws={wsRef} name="LD_hough_thresh" minv={0} maxv={255} step={1} defv={data["LD_hough_thresh"]}/>
                <p>Path finding algoritm</p>
                <Slider ws={wsRef} name="LD_hough_max_line_gap" minv={0} maxv={255} step={1} defv={data["LD_hough_max_line_gap"]}/>
                <Slider ws={wsRef} name="LD_search_points" minv={0} maxv={20} step={1} defv={data["LD_search_points"]}/>
                <Slider ws={wsRef} name="LD_search_interval" minv={0} maxv={60} step={1} defv={data["LD_search_interval"]}/>
                <Slider ws={wsRef} name="LD_search_range" minv={0} maxv={250} step={1} defv={data["LD_search_range"]}/>
                <p>Resolution settings</p>
                <Slider ws={wsRef} name="LD_source_height" minv={240} maxv={1080} step={1} defv={data["LD_source_height"]}/>
                <Slider ws={wsRef} name="LD_source_width" minv={320} maxv={1920} step={1} defv={data["LD_source_width"]}/>
                <Slider ws={wsRef} name="source_height" minv={240} maxv={1080} step={1} defv={data["source_height"]}/>
                <Slider ws={wsRef} name="source_width" minv={320} maxv={1920} step={1} defv={data["source_width"]}/>
                </div>
                <button className="bg-blue-800 rounded-md mx-2 p-2 hover:bg-blue-700 active:bg-blue-900" onClick={() => {
                  setIsOpen(false)
                }}>Close</button>

              </div>
            </div>
          )}

        </div>
      </div>
    );
}
