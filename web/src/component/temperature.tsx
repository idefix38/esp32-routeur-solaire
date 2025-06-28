import { Thermometer } from 'lucide-react';

interface TemperatureProps {
  value: number;
}

export const Temperature = ({ value }: TemperatureProps) => {
  return (
    <div className="overflow-hidden rounded-lg bg-gray-100 shadow">
      <div className="p-5">
        <div className="flex items-center">
          <div className="flex-shrink-0">
            <Thermometer className="h-8 w-8 text-indigo-600" />
          </div>
          <div className="ml-5 w-0 flex-1">
            <dl>
              <dt className="text-sm font-medium text-gray-500 truncate">
                Température
              </dt>
              <dd className="flex items-baseline">
                <div className="text-2xl font-semibold text-gray-900">
                  {value !== null ? `${value}°C` : 'Chargement...'}
                </div>
              </dd>
            </dl>
          </div>
        </div>
      </div>
    </div>
  );
};