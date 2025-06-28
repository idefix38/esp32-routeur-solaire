import { useEffect, useState } from 'preact/hooks';
import { pagePros } from '../app';
import { WifiForm } from '../component/wifiForm';
import { useToast } from '../context/ToastContext';
import { useEsp32Api } from '../hooks/useEsp32Api';

export default function WifiPage( props: pagePros) {
  const { setToast } = useToast();
  const { callApi, loading } = useEsp32Api();
  const [initialValues, setInitialValues] = useState<{ ssid: string; password: string } | null>(null);

  useEffect(() => {
    (async () => {
      const result = await callApi('/getConfig');
      if (result.success && result.data?.wifi) {
        setInitialValues({
          ssid: result.data.wifi.ssid || '',
          password: result.data.wifi.password || '',
        });
      }
    })();
  }, []);

  const handleWifiSubmit = async (data: { ssid: string; password: string }) => {
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
          <WifiForm onSubmit={handleWifiSubmit} loading={loading} initialValues={initialValues} />
        </div>
      </div>
    </div>
  );
}
