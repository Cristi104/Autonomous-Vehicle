import { useEffect, useRef } from "react";
import ENDPOINT_URL from "./data.js";

export default function VideoStream() {
    const canvasRef = useRef(null);
    const wsRef = useRef(null);

    useEffect(() => {
        const canvas = canvasRef.current;
        const ctx = canvas.getContext("2d");

        const resizeCanvas = () => {
            const rect = canvas.getBoundingClientRect();
            canvas.width = rect.width;
            canvas.height = rect.height;
        };

        resizeCanvas();
        window.addEventListener("resize", resizeCanvas);

        wsRef.current = new WebSocket(`ws://${ENDPOINT_URL}/ws/video`);

        wsRef.current.onmessage = (event) => {
            const img = new Image();
            img.src = "data:image/jpeg;base64," + event.data;

            img.onload = () => {
                ctx.clearRect(0, 0, canvas.width, canvas.height);
                ctx.drawImage(img, 0, 0, canvas.width, canvas.height);
            };
        };

        wsRef.current.onerror = (err) => {
            console.error("WebSocket error:", err);
        };

        return () => {
            wsRef.current?.close();
            window.removeEventListener("resize", resizeCanvas);
        };
    }, []);

    return (
        <div className="col-span-4 p-1 col-start-1 row-span-4 row-start-1 w-full h-full">
            <canvas ref={canvasRef} className="rounded-lg w-full h-full block" />
        </div>
    );
}
