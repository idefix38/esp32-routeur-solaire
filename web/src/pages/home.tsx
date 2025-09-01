import { useEffect, useState } from 'preact/hooks';
import { pagePros } from '../app';
import { Card } from '../component/card';
import { useEsp32Api } from '../hooks/useEsp32Api';
import { Thermometer, Zap } from 'lucide-react';

/**
 * Page for displaying the dashboard with the current temperature.
 * It fetches the temperature from the ESP32 device and updates it every 30 seconds.
 * @param props 
 * @returns 
 */
export default function HomePage( props : pagePros) {
  const [temperature, setTemperature] = useState<number | null>(null);
  const [regulatedPower, setRegulatedPower] = useState<number | null>(null);
  const { callApi } = useEsp32Api();

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
        throw new Error('Erreur lors de la récupération de la température');
      }
      setTemperature(parseFloat(response.data.temperature));
      setRegulatedPower(parseFloat(response.data.regulatedPower));
    } catch (error) {
      console.error('Erreur:', error);
    }
  };

  // Déclenche un appel à l'API toutes les 30 secondes pour mettre à jour la température
  useEffect(() => {
    fetchData(); // Récupère la température au chargement de la page
    const interval = setInterval(fetchData, 30000); // Met à jour toutes les 30 secondes
    return () => clearInterval(interval); // Nettoie l'intervalle lors du démontage du composant
  }, []);

  return (
    <div className="container p-8">
      <div className="max-w-3xl mx-auto">
        <h1 className="text-2xl font-bold text-gray-900 mb-6">Tableau de bord</h1>
        <div className="grid grid-cols-1 gap-6 sm:grid-cols-2 lg:grid-cols-3">
          <Card value={temperature ? `${temperature}°C` : null} label="Température" Icon={Thermometer} />
          <Card value={regulatedPower ? `${regulatedPower} W` : "0" } label="Puissance Régulée" Icon={Zap} />
        </div>        
      </div>
    </div>
  );
}