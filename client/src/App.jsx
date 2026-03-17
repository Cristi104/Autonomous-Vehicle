import { useState } from 'react'
import reactLogo from './assets/react.svg'
import viteLogo from '/vite.svg'
import './App.css'
import VideoStream from './VideoStream.jsx'
import Console from './Console.jsx'
// import ControlPanel from './ControlPanel.jsx'
import 'xterm/css/xterm.css';

function App() {
  const [count, setCount] = useState(0)

  return (
    <>
      <div className="gap-4 h-screen grid grid-cols-5 grid-rows-5 p-4">
        <VideoStream />
        {/*<ControlPanel />*/}
        {/*<Console />*/}
      </div>
    </>
  )
}

export default App
