import { Sunrise, Sunset, Trash2 } from "lucide-react";
import DualRangeSlider from "./dualRangeSlider";
import { useState } from "preact/hooks";
import { formatMinuteToTime } from "../helper/time";

interface periodProps {
  canDelete : boolean;
  periodStart: number;
  periodEnd: number;
  sunRise: number;
  sunSet: number;
  onDelete: () => void;
}

/**
 * Composant d'afficahge d'un periode
 * @param param0
 * @returns
 */
export const Period = ({ periodStart, periodEnd, sunRise, sunSet, onDelete , canDelete }: periodProps) => {
  const [start, setPeriodStart] = useState(periodStart);
  const [end, setPeriodEnd] = useState(periodEnd);
  const [mode, setMode] = useState<'auto' | 'on'>('auto');

  const FormatStart = (start: number): string => {
    if (start == sunRise) {
      return "Du lever du soleil";
    } else if (start == sunSet) {
      return "Du coucher du soleil";
    }
    return `De ${formatMinuteToTime(start)}`;
  };

  const FormatEnd = (end: number): string => {
    if (end == sunRise) {
      return "au lever du soleil";
    } else if (end == sunSet) {
      return "au coucher du soleil";
    } else {
      return "à " + formatMinuteToTime(end);
    }
  };

  return (
    <div className="bg-white shadow-md rounded-lg p-6">
      <div className="flex flex-col sm:flex-row sm:items-center sm:justify-between">
        <h3 className="text-sm font-medium leading-6 text-gray-900 mt-4 text-center sm:text-left order-2 sm:order-2 w-full sm:w-auto">
          {FormatStart(start)} {FormatEnd(end)}
        </h3>
        <div className="flex items-center justify-between sm:mt-0 order-1 sm:order-1">
          <div className="relative flex items-center rounded-full p-1 bg-gray-200">
            <button
              type="button"
              className={`relative rounded-full py-1 px-4 text-sm font-medium transition-colors whitespace-nowrap ${mode === 'auto' ? 'bg-white text-gray-900' : 'text-gray-500'}`}
              onClick={() => setMode('auto')}
            >
              Auto
            </button>
            <button
              type="button"
              className={`relative rounded-full py-1 px-4 text-sm font-medium transition-colors whitespace-nowrap ${mode === 'on' ? 'bg-white text-gray-900' : 'text-gray-500'}`}
              onClick={() => setMode('on')}
            >
              On
            </button>
          </div>
           {canDelete && (<button onClick={onDelete} className="text-gray-400 hover:text-red-500 sm:ml-4">
            <Trash2 size={20} />
          </button>)}
        </div>
      </div>
      
        <DualRangeSlider
          min={0}
          max={1440}
          step={5}
          color="#FFC107"
          startValue={start}
          endValue={end}
          sunrise={sunRise}
          sunset={sunSet}
          sunriseIcon={Sunrise}
          sunsetIcon={Sunset}
          onChange={({ start, end }) => {
            setPeriodStart(start);
            setPeriodEnd(end);
          }}
        />      
    </div>
  );
};