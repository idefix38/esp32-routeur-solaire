import { Navbar } from './component/navBar'
import { LocationProvider, Router } from 'preact-iso';
import Home from './pages/home';
import WifiPage from './pages/wifi';
import MqttPage from './pages/mqtt';

export interface pagePros {
  path : string
}


export function App() {
  return (
    <LocationProvider>  
      <Navbar />    
      <Router>
        <Home path="/"></Home>
        <WifiPage path="/wifi"></WifiPage>
        <MqttPage path="/mqtt"></MqttPage>
        <NotFound default />
      </Router>           
    </LocationProvider>
  )
}

type NotFoundProps = {
  default?: boolean;
}

function NotFound(_props: NotFoundProps) {
  return (
    <div className="container p-8">
      <div className="max-w-3xl mx-auto">
        <h1 className="text-2xl font-bold text-gray-900 mb-6">404 - Page non trouvée</h1>
      </div>
    </div>
  );
}
