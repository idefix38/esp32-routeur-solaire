import { Sunrise, Sunset, Trash2 } from "lucide-react";
import DualRangeSlider from "./dualRangeSlider";
import { formatMinuteToTime } from "../helper/time";
import { period } from "../context/configurationContext";

interface periodProps {
 canDelete: boolean;
 periodStart: number;
 periodEnd: number;
 sunRise: number;
 sunSet: number;
 mode: 'auto' | 'on';
 onDelete: () => void;
 onChange: (value: period) => void;
}

/**
* Composant d'afficahge d'un periode
* @param param0
* @returns
*/
export const Period = ({ periodStart, periodEnd, sunRise, sunSet, onDelete, canDelete, onChange, mode }: periodProps) => {

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
   {/* Conteneur principal pour la disposition */}
   <div className="flex flex-col gap-4 sm:flex-row sm:items-center sm:justify-between">

    {/* 1. Ligne d'informations (Mobile: Centré | Desktop: Premier, à gauche) */}
    <div className="flex sm:flex-none justify-center sm:justify-start w-full sm:w-auto order-1 sm:order-1">
     <h3 className="text-sm font-medium leading-6 text-gray-900 text-center sm:text-left">
      {FormatStart(periodStart)} {FormatEnd(periodEnd)}
     </h3>
     {/* Icône Trash à côté des heures sur mobile */}
     {canDelete && (<button onClick={onDelete} className="text-gray-400 hover:text-red-500 ml-4 sm:hidden">
      <Trash2 size={20} />
     </button>)}
    </div>

    {/* 2. Switch Auto/On et Icône Trash (Mobile: Centré | Desktop: À droite) */}
    <div className="grow flex justify-center sm:justify-start sm:mt-0 w-full order-2 sm:order-2">
     {/* Switch Auto/On */}
     <div className="relative flex size-min items-center rounded-full p-1 bg-gray-200">
      <button
       type="button"
       className={`relative rounded-full py-1 px-4 text-sm font-medium transition-colors whitespace-nowrap ${mode === 'auto' ? 'bg-white text-gray-900' : 'text-gray-500'}`}
       onClick={() => onChange({ start: periodStart, end: periodEnd, mode: 'auto' })}
      >
       Auto
      </button>
      <button
       type="button"
       className={`relative rounded-full py-1 px-4 text-sm font-medium transition-colors whitespace-nowrap ${mode === 'on' ? 'bg-white text-gray-900' : 'text-gray-500'}`}
       onClick={() => onChange({ start: periodStart, end: periodEnd, mode: 'on' })}
      >
       On
      </button>
     </div>    
    </div>
    <div className="hidden sm:block justify-end flex-none mt-4 sm:mt-0  sm:order-2">
       {/* Icône Trash sur Desktop (masquée sur mobile car déplacée) */}
     {canDelete && (<button onClick={onDelete} className="text-gray-400 hover:text-red-500 ml-4">
      <Trash2 size={20} />
     </button>)}
    </div>
   </div>
   
   {/* Slider toujours en bas */}
   <DualRangeSlider
    min={0}
    max={1440}
    step={1}
    color="#FFC107"
    startValue={periodStart}
    endValue={periodEnd}
    sunrise={sunRise}
    sunset={sunSet}
    sunriseIcon={Sunrise}
    sunsetIcon={Sunset}
    onChange={({ start, end }) => {
     onChange({ start : start, end: end, mode: mode , startSunrise: start === sunRise, startSunset: start === sunSet, endSunrise: end === sunRise, endSunset: end === sunSet });
    }}
   />
  </div>
 );
};