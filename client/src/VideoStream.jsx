import { useEffect, useRef } from "react";
import ENDPOINT_URL from "./data.js";

export default function VideoStream() {
    const canvasRef = useRef(null);
    const wsRef = useRef(null);

    useEffect(() => {
        const canvas = canvasRef.current;
        const ctx = canvas.getContext("2d");

        wsRef.current = new WebSocket(`ws://${ENDPOINT_URL}/ws/video`);

        wsRef.current.onmessage = (event) => {
            const img = new Image();
            img.src = "data:image/jpeg;base64," + event.data;

            img.onload = () => {
                canvas.width = img.width;
                canvas.height = img.height;
                ctx.drawImage(img, 0, 0);
            };
        };

        wsRef.current.onerror = (err) => {
            console.error("WebSocket error:", err);
        };

        return () => {
            wsRef.current?.close();
        };
    }, []);

    return (
        <div>
            <canvas ref={canvasRef} />
        </div>
    );
}