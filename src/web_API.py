from fastapi import FastAPI, WebSocket
from starlette.websockets import WebSocketState
import numpy as np
import cv2 as cv
import uvicorn
import base64
import asyncio
import threading

class web_API:
    def __init__(self):
        self.app = FastAPI()
        self.frame = np.ones((480, 640), dtype=np.uint8)
        self.frame_mutex = threading.Lock()
        @self.app.websocket("/ws/video")
        async def video_stream(websocket: WebSocket):
            await websocket.accept()

            try:
                while True:
                    with self.frame_mutex:
                        _, buffer = cv.imencode(".jpg", self.frame)
                    jpg_as_text = base64.b64encode(buffer).decode("utf-8")

                    await websocket.send_text(jpg_as_text)
                    await asyncio.sleep(0.03)

            except Exception:
                pass
            finally:
                if websocket.application_state == WebSocketState.CONNECTED:
                    await websocket.close()
        self.run_api_thread()

    def send_image(self, frame):
        with self.frame_mutex:
            self.frame = frame

    def run_api(self):
        uvicorn.run(self.app, host="0.0.0.0", port=8000)

    def run_api_thread(self):
        threading.Thread(target=self.run_api, daemon=True).start()
