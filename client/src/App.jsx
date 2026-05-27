import { useState } from 'react'
import reactLogo from './assets/react.svg'
import viteLogo from '/vite.svg'
// import './index.css'
// import './App.css'
import VideoStream from './VideoStream.jsx'
import Console from './Console.jsx'
import ControlPanel from './ControlPanel.jsx'
// import 'xterm/css/xterm.css'

function App() {
  const [count, setCount] = useState(0)

  return (
    <>
      <div className="h-screen bg-stone-900">
        <VideoStream />
        <ControlPanel />
        {/*<Console />*/}
      </div>
    </>
  )
}

export default App
