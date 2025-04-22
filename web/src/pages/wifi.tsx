import { pagePros } from '../app';
import { WifiForm } from '../component/wifiForm';

export default function WifiPage( props: pagePros) {
  const handleWifiSubmit = (data: { ssid: string; password: string }) => {
    console.log('Données WiFi soumises:', data);
    // Ici, vous pourrez ajouter la logique pour sauvegarder les paramètres WiFi
  };

  return (
    <div className="container p-8">
      <div className="divide-y divide-gray-200 overflow-hidden rounded-lg bg-white shadow">
        <div className="px-4 py-5 sm:px-6 bg-indigo-600">
          <h1 className="text-xl font-semibold text-white">Paramètres WiFi</h1>
        </div>
        <div className="px-4 py-5 sm:p-6 bg-gray-100">
          <WifiForm onSubmit={handleWifiSubmit} />
        </div>
      </div>
    </div>
  );
}
