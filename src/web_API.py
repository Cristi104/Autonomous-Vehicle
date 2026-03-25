from fastapi import FastAPI, WebSocket
from starlette.websockets import WebSocketState
import numpy as np
import cv2 as cv
import uvicorn
import base64
import asyncio
import threading
from src import config
from fastapi.middleware.cors import CORSMiddleware


class web_API:
    def __init__(self):
        self.app = FastAPI()
        self.frame = np.ones((480, 640), dtype=np.uint8)
        self.frame_mutex = threading.Lock()
        self.control_queue = []
        self.control_queue_mutex = threading.Lock()

        self.app.add_middleware(
            CORSMiddleware,
            allow_origins=["*"],
            allow_credentials=True,
            allow_methods=["*"],
            allow_headers=["*"],
        )

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

        @self.app.websocket("/ws/control")
        async def control_panel(websocket: WebSocket):
            await websocket.accept()
            try:

                while True:
                    data = await websocket.receive_text()
                    print(data)
                    self.control_queue_push(data)

            except Exception:
                pass
            finally:
                if websocket.application_state == WebSocketState.CONNECTED:
                    await websocket.close()

        @self.app.get("/config")
        async def get_config():
            return{
                    "speed": config.speed,
                    "kp": config.kp,
                    "ki": config.ki,
                    "kd": config.kd,
                    "kdef": config.kdef,
            }

        self.run_api_thread()

    def send_image(self, frame):
        with self.frame_mutex:
            self.frame = frame

    def control_queue_push(self, message):
        with self.control_queue_mutex:
            self.control_queue.append(message)
    
    def control_queue_pop(self):
        with self.control_queue_mutex:
            if len(self.control_queue) > 0:
                message = self.control_queue.pop(0)
            else:
                message = "[NONE]"
        return message

    def run_api(self):
        uvicorn.run(self.app, host="0.0.0.0", port=8000)

    def run_api_thread(self):
        threading.Thread(target=self.run_api, daemon=True).start()
