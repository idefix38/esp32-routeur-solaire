
import { pagePros } from '../app';
import { WifiForm } from '../component/wifiForm';
import { useToast } from '../context/ToastContext';
import { useEsp32Api } from '../hooks/useEsp32Api';
import { useConfig, wifiConfig } from '../context/configurationContext';


/**
 * Page for configuring WiFi settings.
 * It fetches the current WiFi settings from the ESP32 device and allows the user to update them.
 * The form includes fields for SSID and password, and displays a success or error message upon submission.
 */
export default function WifiPage( props: pagePros) {
  const { setToast } = useToast();
  const { callApi, loading } = useEsp32Api();
  const config = useConfig();
   
  // Handles the form submission for WiFi settings.
  const handleWifiSubmit = async (data: wifiConfig) => {
    console.log("Wifi settings ", data)
    const result = await callApi('/saveWifiSettings', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(data)
    });
    if (result.success) {
      setToast({message: 'Paramètres WiFi enregistrés avec succès', type: 'success'});
    } else {
      setToast({message: 'Erreur lors de l\'enregistrement des paramètres WiFi', type: 'error'});
    }
  };

  return (
    <div className="container p-8">
      <div className="divide-y divide-gray-200 overflow-hidden rounded-lg bg-white shadow">
        <div className="px-4 py-5 sm:px-6 bg-indigo-600">
          <h1 className="text-xl font-semibold text-white">Paramètres WiFi</h1>
        </div>
        <div className="px-4 py-5 sm:p-6 bg-gray-100">
          <div className={(!config.value?.wifi || loading) ? 'pointer-events-none opacity-50 relative' : ''}>
            <WifiForm onSubmit={handleWifiSubmit} loading={loading} initialValues={config.value?.wifi} />
            {(loading) && (
              <div className="absolute inset-0 flex items-center justify-center z-10">
                <span className="text-indigo-600 font-semibold text-lg animate-pulse">Chargement...</span>
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}
