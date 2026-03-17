import { useEffect, useRef } from "react";
import { Terminal } from 'xterm';


export default function Console() {
  const termObjRef = useRef(null);
  const termRef = useRef(null);
  const wsRef = useRef(null);
  const initializedRef = useRef(false);

  useEffect(() => {
    let term
    wsRef.current = new WebSocket("ws://192.168.1.157:8000/ws/log");
    if (!initializedRef.current){
      initializedRef.current = true;
      termObjRef.current = new Terminal();
      termObjRef.current.open(termRef.current);
      termObjRef.current.resize(80, 50)

      termObjRef.current.onData((event) => {
        wsRef.current.send(event)
      });
    }

    wsRef.current.onmessage = (event) => {
      console.log(event.data)
      termObjRef.current.write(event.data);
    };


    wsRef.current.onerror = (err) => {
      console.error("WebSocket error:", err);
    };

    return () => {
      wsRef.current?.close();
    };
  }, []);
  
  return(
    <>
      <div id="terminal" className="h-full w-full col-start-4 row-start-1 row-span-5 col-span-2" ref={termRef}></div>
    </>
  );
}
