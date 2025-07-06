import { Navbar } from './component/navBar'
import { lazy, LocationProvider, Router } from 'preact-iso';
import { ToastProvider, useToast } from './context/ToastContext';
import { Toast } from './component/Toast';
import SolarPage from './pages/solar';

const Home = lazy(() => import('./pages/home'));
const WifiPage = lazy(() => import('./pages/wifi'));;
const MqttPage  = lazy(() => import('./pages/mqtt'));;

export interface pagePros {
  path : string
}


export function App(props: any) {
  return (
    <LocationProvider>
      <ToastProvider>
        <Navbar />
        <ToastContainer />
        <Router>
          <Home path="/" />
          <WifiPage path="/wifi" />
          <MqttPage path="/mqtt" />
          <SolarPage path="/solar" />
          <NotFound default />
        </Router>
      </ToastProvider>
    </LocationProvider>
  );
}

function ToastContainer() {
  const { toast, setToast } = useToast();
  if (!toast) return null;
  return <Toast message={toast.message} type={toast.type} onClose={() => setToast(null)} />;
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
