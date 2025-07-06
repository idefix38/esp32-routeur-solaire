import { useEffect, useState } from 'preact/hooks';
import { pagePros } from '../app';
import { Temperature } from '../component/temperature';
import { useEsp32Api } from '../hooks/useEsp32Api';

/**
 * Page for displaying the dashboard with the current temperature.
 * It fetches the temperature from the ESP32 device and updates it every 30 seconds.
 * @param props 
 * @returns 
 */
export default function HomePage( props : pagePros) {
  const [temperature, setTemperature] = useState<number | null>(null);
  const { callApi } = useEsp32Api();

  /**
   * Fetch the current temperature from the ESP32 device.
   */
  const fetchTemperature = async () => {
    try {
      const response = await callApi('/getTemperature', 
        {method: 'GET',
        headers: { 'Content-Type': 'application/json' }}
      );
      if (!response.success) {
        throw new Error('Erreur lors de la récupération de la température');
      }
      setTemperature(parseFloat(response.data.temperature));
    } catch (error) {
      console.error('Erreur:', error);
    }
  };

  // Déclenche un appel à l'API toutes les 30 secondes pour mettre à jour la température
  useEffect(() => {
    fetchTemperature(); // Récupère la température au chargement de la page
    const interval = setInterval(fetchTemperature, 30000); // Met à jour toutes les 30 secondes
    return () => clearInterval(interval); // Nettoie l'intervalle lors du démontage du composant
  }, []);

  return (
    <div className="container p-8">
      <div className="max-w-3xl mx-auto">
        <h1 className="text-2xl font-bold text-gray-900 mb-6">Tableau de bord</h1>
        <div className="grid grid-cols-1 gap-6 sm:grid-cols-2 lg:grid-cols-3">
          <Temperature value={temperature ?? 0} />
        </div>        
      </div>
    </div>
  );
}
