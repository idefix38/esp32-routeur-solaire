import { Loader } from "lucide-react";
import { boilerConfig, period } from "../context/configurationContext";
import { useRef, useEffect, useState } from "preact/hooks";
import { FormEvent } from "react";
import { Period } from "./period";

interface boilerFormProps {
  onSubmit: (boilerSettings?: boilerConfig) => Promise<any>;
  loading?: boolean;
  boilerSettings?: boilerConfig;
}

type PeriodType = {
  id: number;
  start: number;
  end: number;
  mode: "auto" | "on";
};

let nextId = 0;

export const BoilerForm = ({ onSubmit, boilerSettings, loading }: boilerFormProps) => {

  const temperatureRef = useRef<HTMLInputElement>(null);
  const [periods, setPeriods] = useState<PeriodType[]>([]);

  useEffect(() => {
    let initialPeriods: PeriodType[] = [];
    if (boilerSettings) {
      if (temperatureRef.current) {
        temperatureRef.current.value = boilerSettings.temperature?.toString() || '50';
      }
      if (boilerSettings.periods && boilerSettings.periods.length > 0) {
        initialPeriods = boilerSettings.periods.map((p) => ({ ...p, id: nextId++ }));
      } else if (boilerSettings.periodStart !== undefined && boilerSettings.periodEnd !== undefined) {
        initialPeriods = [{ id: nextId++, start: boilerSettings.periodStart, end: boilerSettings.periodEnd, mode: 'auto' }];
      }
    }

    if (initialPeriods.length === 0) {
      initialPeriods = [{ id: nextId++, start: 6 * 60, end: 18 * 60, mode: 'auto' }];
    }
    
    setPeriods(initialPeriods);
  }, [boilerSettings]);

  // Fonction pour mettre à jour une période
  const handlePeriodChange = (id: number, newValues: { start: number; end: number; mode: 'auto' | 'on' }) => {
    setPeriods(periods.map(p => p.id === id ? { ...p, ...newValues } : p));
  };

  // Fonction pour ajouter une nouvelle période
  const addPeriod = () => {
    setPeriods([...periods, { id: nextId++, start: 8 * 60, end: 17 * 60, mode: 'auto' }]);
  };

  // Fonction pour supprimer une période
  const deletePeriod = (id: number) => {
    if (periods.length > 1) {
      setPeriods(periods.filter(p => p.id !== id));
    }
  };

  // Donnée à envoyer
  const handleSubmit = (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    if (temperatureRef.current && boilerSettings) {
      const newSettings: boilerConfig = {
        ...boilerSettings,
        temperature: parseFloat(temperatureRef.current.value),
        periods: periods.map(({ start, end, mode }) => ({ start, end, mode }))
      };
      onSubmit(newSettings);
    }
  }

  return (
    <form onSubmit={handleSubmit} className="space-y-6">
      <div>
        <label htmlFor="boilerTemp" className="block text-sm font-medium text-gray-700">Température cible du chauffe-eau (C°)</label>
        <input
          id="boilerTemp"
          name="boilerTemp"
          type="number"
          ref={temperatureRef}
          className="mt-1 block w-full rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500 text-lg px-4 py-3"
          placeholder="50 °C"
          required
        />
      </div>
      <label className="block text-sm font-medium text-gray-700">Périodes de chauffe</label>
      <div className="space-y-4">
        {periods.map((period) => (
          <Period
            key={period.id}
            periodStart={period.start}
            periodEnd={period.end}
            sunRise={6 * 60} // This should probably come from props/context
            sunSet={18 * 60} // This should probably come from props/context
            canDelete={periods.length > 1}
            mode={period.mode}
            onDelete={() => deletePeriod(period.id)}
            onChange={(newValues) => handlePeriodChange(period.id, newValues)}
          />
        ))}
      </div>
      <div className="space-x-4">

      
      <button 
        type="button" 
        onClick={addPeriod}
        className="inline-flex justify-center rounded-md border border-gray-300 bg-white py-2 px-4 text-sm font-medium text-gray-700 shadow-sm hover:bg-gray-50 focus:outline-none focus:ring-2 focus:ring-indigo-500 focus:ring-offset-2"
      >
        + Période
      </button>

      <button
        type="submit"
        className="inline-flex justify-center rounded-md border border-transparent bg-indigo-600 py-2 px-4 text-sm font-medium text-white shadow-sm hover:bg-indigo-700 focus:outline-none focus:ring-2 focus:ring-indigo-500 focus:ring-offset-2"
        disabled={loading}
      >
        {loading && (
          <Loader className="mr-2 h-5 w-5 animate-spin text-white" />
        )}
        Enregistrer
      </button>
      </div>
    </form>
  );
}