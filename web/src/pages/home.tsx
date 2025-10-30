import { pagePros } from '../app';
import { Card } from '../component/card';
import { Thermometer, Zap,Sun } from 'lucide-react';
import { useConfig } from '../context/configurationContext';
import { useEsp32WebSocket } from '../hooks/useEsp32WebSocket';
import { formatMinuteToTime } from '../helper/time';

/**
 * Page for displaying the dashboard with real-time data from the ESP32.
 * It uses a WebSocket connection to receive updates.
 * @param props 
 * @returns 
 */
export default function HomePage(props: pagePros) {
  // Utilise le hook WebSocket pour obtenir les données et le statut en temps réel
  const { data } = useEsp32WebSocket();
  const { value: config } = useConfig();

  const isTemperatureTargetReached =
    data.temperature !== undefined &&
    config?.boiler?.temperature !== undefined &&
    data.temperature >= config.boiler.temperature;

  return (
    <div className="container p-8">
      <div className="max-w-3xl mx-auto">
        <h1 className="text-2xl font-bold text-gray-900 mb-6">Tableau de bord</h1>
        <div className="grid grid-cols-1 gap-6 sm:grid-cols-2 lg:grid-cols-3">
          <Card 
            value={data.temperature !== undefined ? `${data.temperature.toFixed(1)}°C` : "..."} 
            label="Température" 
            Icon={Thermometer} 
            showCheck={isTemperatureTargetReached}  
          />          
          <Card 
            value={data.triacOpeningPercentage !== undefined ? `${data.triacOpeningPercentage.toFixed(0)} %` : "..."} 
            label="Ouverture Triac" 
            Icon={Zap} 
          />
          <Card 
            value={config?.solar.sunRiseMinutes !== undefined && config.solar.sunSetMinutes !== undefined ? `${formatMinuteToTime( config.solar.sunRiseMinutes)} - ${formatMinuteToTime(config.solar.sunSetMinutes)}` : "..."} 
            label="Lever / Coucher"
            Icon={Sun} 
          />
        </div>        
      </div>
    </div>
  );
}