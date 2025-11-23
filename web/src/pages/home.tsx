import { pagePros } from '../app';
import { Card } from '../component/card';
import HistoryChart from '../component/historyChart';
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
  

  return (
    <div className="container p-8">
      <div className="max-w-3xl mx-auto">
        <h1 className="text-2xl font-bold text-gray-900 mb-6">Tableau de bord</h1>
        <div className="grid grid-cols-1 gap-6">
          {/* Temperature card with history chart */}
          <Card
            value={data.temperature !== undefined ? `${data.temperature.toFixed(1)}°C` : "..."}
            label="Température"
            Icon={Thermometer}
            showCheck={data.temperatureReached ?? false}
          >
            {/* Compute temperature history bounds and pass to HistoryChart */}
            {data.temperatureHistory && data.temperatureHistory.length > 1 ? (
              <div className="mt-4 h-40">
                <HistoryChart
                  data={data.temperatureHistory}
                  minY={Math.floor(Math.min(...data.temperatureHistory.map((p) => p.value)) / 10) * 10}
                  maxY={Math.ceil(Math.max(...data.temperatureHistory.map((p) => p.value)) / 10) * 10}
                  lineClass='stroke-indigo-600'
                  labelClass='text-stone-950'
                  labelTemplate='{time} - {value} °C'
                  xStep={(() => {
                    const arr = data.temperatureHistory || [];
                    const len = arr.length;
                    if (len <= 6) return 4 * 3600;
                    if (len <= 24) return 2 * 3600;
                    return 3600;
                  })()}
                />
              </div>
            ) : null}
          </Card>

          {/* Triac card with history chart */}
          <Card
            value={data.triacOpeningPercentage !== undefined ? `${data.triacOpeningPercentage.toFixed(0)} %` : "..."}
            label="Ouverture Triac"
            Icon={Zap}
          >
            {data.triacHistory && data.triacHistory.length > 1 ? (
              <div className="mt-4 h-40">
                <HistoryChart data={data.triacHistory} minY={0} maxY={100}
                  lineClass='stroke-indigo-600'
                  labelClass='text-stone-950'
                  yStep={20}
                  labelTemplate='{time} - {value} %'
                  xStep={(() => {
                    const arr = data.triacHistory || [];
                    const len = arr.length;
                    if (len <= 6) return 4 * 3600;
                    if (len <= 24) return 2 * 3600;
                    return 3600;
                  })()} />
              </div>
            ) : null}
          </Card>

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