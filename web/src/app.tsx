import { Navbar } from './component/navBar'
import { lazy, LocationProvider, Router } from 'preact-iso';
import { ToastProvider, useToast } from './context/ToastContext';
import { Toast } from './component/Toast';

const Home = lazy(() => import('./pages/home'));
const WifiPage = lazy(() => import('./pages/wifi'));;
const MqttPage  = lazy(() => import('./pages/mqtt'));;

export interface pagePros {
  path : string
}


export function App(props: any) {
  const { toast, setToast } = useToast();
  return (
    <LocationProvider>
    <ToastProvider>
      <Navbar />          
      {toast && (
        <Toast message={toast.message} type={toast.type} onClose={() => setToast(null)} />
      )}
      <Router>
        <Home path="/"></Home>
        <WifiPage path="/wifi"></WifiPage>
        <MqttPage path="/mqtt"></MqttPage>
        <NotFound default />
      </Router>  
    </ToastProvider>
    </LocationProvider>
  );
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
