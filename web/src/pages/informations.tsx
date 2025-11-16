import { pagePros } from "../app";
import { useToast } from "../context/ToastContext";
import { useEsp32Api } from "../hooks/useEsp32Api";
import { useEsp32WebSocket } from "../hooks/useEsp32WebSocket";
import { useState, useEffect } from "preact/hooks";

interface UpdateInfo {
  new_version: string;
  // ... autres champs potentiels
}

export default function InformationsPage(props: pagePros) {
  const { data } = useEsp32WebSocket();
  const { callApi, loading } = useEsp32Api();
  const { setToast } = useToast();
  const [newVersionInfo, setNewVersionInfo] = useState<UpdateInfo | null>(null);
  const [isUpdating, setIsUpdating] = useState(false);
  const [targetVersion, setTargetVersion] = useState<string | null>(null);

  // A new version is considered available if our check found one,
  // or if the initial WebSocket data says so.
  const newVersionAvailable = newVersionInfo || (data?.newFirmwareVersion && data.newFirmwareVersion !== data.currentFirmwareVersion);
  const latestVersion = newVersionInfo?.new_version || data?.newFirmwareVersion || data?.currentFirmwareVersion;

  useEffect(() => {
    // If we are in "updating" mode and have a target version
    if (isUpdating && targetVersion) {
        // And the current version from the WebSocket matches our target
        if (data?.currentFirmwareVersion === targetVersion) {
            // Redirect and refresh
            window.location.href = '/';
        }
    }
  }, [data, isUpdating, targetVersion]);

  const handleCheckForUpdate = async () => {
    const result = await callApi('/api/update/check', { method: 'GET' });
    if (result.success && result.data) {
      // Check if the response is not an empty object
      if (Object.keys(result.data).length > 0 && result.data.new_version) {
        if (result.data.new_version !== data?.currentFirmwareVersion) {
          setNewVersionInfo(result.data);          
        } else {
          setNewVersionInfo(null);
          setToast({ message: "Vous avez la dernière version.", type: 'success' });
        }
      } else {
        setNewVersionInfo(null);
        setToast({ message: "Vous avez la dernière version.", type: 'success' });
      }
    } else {
      setToast({ message: "Erreur lors de la vérification de mise à jour.", type: 'error' });
    }
  };

  const handleStartUpdate = async () => {
    if (latestVersion) {
        setTargetVersion(latestVersion);
    }
    const result = await callApi('/api/update/start', { method: 'POST' });
    if (result.success) {
      setIsUpdating(true);
    } else {
      setToast({ message: "Erreur lors du lancement de la mise à jour.", type: 'error' });
      setTargetVersion(null); // Reset on failure
    }
  };

  return (
    <div className="container p-8">
      <div className="max-w-3xl mx-auto">
        <h1 className="text-2xl font-bold text-gray-900 mb-6">Informations</h1>
        <div className="bg-white shadow overflow-hidden sm:rounded-lg">
          <ul className="divide-y divide-gray-200">
            <li className="px-4 py-5 sm:px-6">
              <div className="flex items-center justify-between">
                <p className="text-sm font-medium text-gray-500">URL du projet</p>
                <a href="https://github.com/idefix38/esp32-routeur-solaire" target="_blank" rel="noopener noreferrer" className="text-sm text-blue-600 hover:underline">
                  https://github.com/idefix38/esp32-routeur-solaire
                </a>
              </div>
            </li>
            <li className="px-4 py-5 sm:px-6">
              <div className="flex items-center justify-between">
                <p className="text-sm font-medium text-gray-500">Version du firmware</p>
                <p className="text-sm text-gray-900">{data?.currentFirmwareVersion || 'N/A'}</p>
              </div>
            </li>
            <li className="px-4 py-5 sm:px-6">
              <div className="flex items-center justify-between">
                <p className="text-sm font-medium text-gray-500">Dernière version disponible</p>
                <p className="text-sm text-gray-900">{latestVersion || 'N/A'}</p>
              </div>
            </li>
            <li className="px-4 py-5 sm:px-6">
              <div className="flex items-center justify-center">
                {isUpdating ? (
                    <div className="flex items-center">
                        <div className="animate-spin rounded-full h-6 w-6 border-b-2 border-blue-500"></div>
                        <p className="ml-3 text-sm text-gray-600">Chargement en cours, l'ESP32 redémarrera après la mise à jour.</p>
                    </div>
                ) : newVersionAvailable ? (
                  <button
                    onClick={handleStartUpdate}
                    disabled={loading}
                    className="w-full inline-flex justify-center py-2 px-4 border border-transparent shadow-sm text-sm font-medium rounded-md text-white bg-green-600 hover:bg-green-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-green-500 disabled:bg-gray-400"
                  >
                    {loading ? 'Mise à jour en cours...' : `Mettre à jour vers ${latestVersion}`}
                  </button>
                ) : (
                  <button
                    onClick={handleCheckForUpdate}
                    disabled={loading}
                    className="w-full inline-flex justify-center py-2 px-4 border border-transparent shadow-sm text-sm font-medium rounded-md text-white bg-blue-600 hover:bg-blue-700 focus:outline-none focus:ring-2 focus:ring-offset-2 focus:ring-blue-500 disabled:bg-gray-400"
                  >
                    {loading ? 'Vérification...' : 'Vérifier les mises à jour'}
                  </button>
                )}
              </div>
            </li>
          </ul>
        </div>
      </div>
    </div>
  );
}