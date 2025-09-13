import { useEffect, useState } from 'preact/hooks';
import { pagePros } from '../app';
import { Card } from '../component/card';
import { useEsp32Api } from '../hooks/useEsp32Api';
import { Thermometer, Zap } from 'lucide-react';
import { useConfig } from '../context/configurationContext';

/**
 * Page for displaying the dashboard with the current temperature.
 * It fetches the temperature from the ESP32 device and updates it every 30 seconds.
 * @param props 
 * @returns 
 */
export default function HomePage( props : pagePros) {
  const [temperature, setTemperature] = useState<number | null>(null);
  const [triacOpeningPercentage, setTriacOpeningPercentage] = useState<number | null>(null);
  const { callApi } = useEsp32Api();
  const { value: config } = useConfig();

  /**
   * Fetch the current temperature from the ESP32 device.
   */
  const fetchData = async () => {
    try {
      const response = await callApi('/getData', 
        {method: 'GET',
        headers: { 'Content-Type': 'application/json' }}
      );
      if (!response.success) {
        throw new Error('Erreur lors de la récupération des données');
      }
      setTemperature(parseFloat(response.data.temperature));
      setTriacOpeningPercentage(parseFloat(response.data.triacOpeningPercentage));
    } catch (error) {
      console.error('Erreur:', error);
    }
  };

  // Déclenche un appel à l'API toutes les 5 secondes pour mettre à jour les données
  useEffect(() => {
    fetchData(); // Récupère les données au chargement de la page
    const interval = setInterval(fetchData, 5000); // Met à jour toutes les 5 secondes
    return () => clearInterval(interval); // Nettoie l'intervalle lors du démontage du composant
  }, []);

  const isTemperatureTargetReached =
  temperature !== null &&
  config?.boiler?.temperature !== undefined &&
  temperature >= config.boiler.temperature;

  return (
    <div className="container p-8">
      <div className="max-w-3xl mx-auto">
        <h1 className="text-2xl font-bold text-gray-900 mb-6">Tableau de bord</h1>
        <div className="grid grid-cols-1 gap-6 sm:grid-cols-2 lg:grid-cols-3">
          <Card value={temperature ? `${temperature}°C` : null} label="Température" Icon={Thermometer} showCheck={isTemperatureTargetReached}  />
          <Card value={triacOpeningPercentage ? `${triacOpeningPercentage.toFixed(0)} %` : "0 %" } label="Ouverture Triac" Icon={Zap} />
        </div>        
      </div>
    </div>
  );
}