import { Navbar } from './component/navBar'
import { LocationProvider, Router, lazy } from 'preact-iso';
import { ToastProvider, useToast } from './context/ToastContext';
import { Toast } from './component/Toast';
import { ConfigProvider } from './context/configurationContext';

const Home = lazy(() => import('./pages/home'));
const WifiPage = lazy(() => import('./pages/wifi'));
const MqttPage  = lazy(() => import('./pages/mqtt'));
const SolarPage  = lazy(() => import('./pages/solar'));
const BoilerPage = lazy(()=>import('./pages/boiler'))


export interface pagePros {
  path : string
}


export function App(props: any) {
  return (
    <LocationProvider>
      <ConfigProvider>
      <ToastProvider>
        <Navbar />
        <ToastContainer />
        <Router>
          <Home path="/" />
          <WifiPage path="/wifi" />
          <MqttPage path="/mqtt" />
          <SolarPage path="/solar" />
          <BoilerPage path="/boiler"/>
          <NotFound default />
        </Router>
      </ToastProvider>
      </ConfigProvider>
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
