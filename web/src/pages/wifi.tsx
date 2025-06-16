import { pagePros } from '../app';
import { WifiForm } from '../component/wifiForm';
import { Toast } from '../component/Toast';
import { useState } from 'react';

export default function WifiPage( props: pagePros) {
  const [toast, setToast] = useState<{ message: string; type: 'success' | 'error' } | null>(null);

  const handleWifiSubmit = async (data: { ssid: string; password: string }) => {
    console.log('Données WiFi soumises:', data);

    try {
      const response = await fetch('/saveWifiSettings', {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify(data),
      });

      if (!response.ok) {
        throw new Error('Erreur lors de la soumission des données WiFi');
      }

      const result = await response.json();
      console.log('Réponse du serveur:', result);
      setToast({ message: 'Paramètres WiFi enregistrés avec succès !', type: 'success' });
    } catch (error) {
      console.error('Erreur:', error);
      setToast({ message: 'Erreur lors de l’enregistrement des paramètres WiFi.', type: 'error' });
    }
  };

  return (
    <div className="container p-8">
      {toast && (
        <Toast
          message={toast.message}
          type={toast.type}
          onClose={() => setToast(null)}
        />
      )}
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
