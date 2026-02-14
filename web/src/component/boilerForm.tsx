import { Loader } from "lucide-react";
import { boilerConfig, period } from "../context/configurationContext";
import { useRef, useEffect, useState } from "preact/hooks";
import { FormEvent } from "react";
import { Period } from "./period";

interface boilerFormProps {
  onSubmit: (boilerSettings?: boilerConfig) => Promise<any>;
  loading?: boolean;
  boilerSettings?: boilerConfig;
  sunRiseMinutes: number;
  sunSetMinutes: number;
}

type PeriodType = period & {
  id: number;
};

let nextId = 0;

export const BoilerForm = ({ onSubmit, boilerSettings, loading, sunRiseMinutes,sunSetMinutes }: boilerFormProps) => {
  
  const temperatureRef = useRef<HTMLInputElement>(null);
  const [periods, setPeriods] = useState<PeriodType[]>([]);
  const [currentMode, setCurrentMode] = useState(boilerSettings?.mode?.toLowerCase() || 'auto');
  const [triacOpening, setTriacOpening] = useState(boilerSettings?.triacOpening || 50);

  const handleModeChange = (e: Event) => {
    setCurrentMode((e.target as HTMLInputElement).value);
  };

  // Affichage des valeurs par défaut
  useEffect(() => {
    let initialPeriods: PeriodType[] = [];
    if (boilerSettings) {
      setCurrentMode(boilerSettings.mode.toLowerCase() || 'auto');
      setTriacOpening(boilerSettings.triacOpening || 50);
      if (temperatureRef.current) {
        temperatureRef.current.value = boilerSettings.temperature?.toString() || '50';
      }
      if (boilerSettings.periods && boilerSettings.periods.length > 0) {
        initialPeriods = boilerSettings.periods.map((p) => ({ ...p, id: nextId++ }));
      }
    }
    if (initialPeriods.length === 0) {
      // Initialize with a default period if none exist
      initialPeriods = [{ id: nextId++, start: sunRiseMinutes, end: sunSetMinutes, mode: 'auto' }];
    }

    setPeriods(initialPeriods);
  }, [boilerSettings]);

  // Fonction pour mettre à jour une période
  const handlePeriodChange = (id: number, newValues: { start: number; end: number; mode: 'auto' | 'on' }) => {
    setPeriods(periods.map(p => p.id === id ? { ...p, ...newValues } : p));
  };

  // Fonction pour ajouter une nouvelle période
  const addPeriod = () => {
    setPeriods([...periods, { id: nextId++, start: sunRiseMinutes, end: sunSetMinutes, mode: 'auto' }]);
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

    const newSettings: boilerConfig = {
      mode: currentMode,
      temperature: parseFloat(temperatureRef.current?.value || "50"),
      periods: periods.map(({ start, end, mode, startSunrise, startSunset, endSunrise, endSunset }) => ({ start, end, mode, startSunrise, startSunset, endSunrise, endSunset })),
      triacOpening: currentMode === 'manual' ? triacOpening : undefined
    };
    onSubmit(newSettings);
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
      {/* // Mode */}
      <div>
        <span className="block text-sm font-medium text-gray-700 mb-4">Mode de fonctionnement</span>
        <label className="inline-flex items-center mr-4">
          <input
            type="radio"
            name="mode"
            value="auto"
            className="form-radio text-indigo-600"
            checked={currentMode === 'auto'}
            onChange={handleModeChange}
          />
          <span className="ml-2">Personnalisé</span>
        </label>
        <label className="inline-flex items-center mr-4">
          <input
            type="radio"
            name="mode"
            value="on"
            className="form-radio text-indigo-600"
            checked={currentMode === 'on'}
            onChange={handleModeChange}
          />
          <span className="ml-2">On (Marche forcé)</span>
        </label>
        <label className="inline-flex items-center mr-4">
          <input
            type="radio"
            name="mode"
            value="manual"
            className="form-radio text-indigo-600"
            checked={currentMode === 'manual'}
            onChange={handleModeChange}
          />
          <span className="ml-2">Manuel</span>
        </label>
        <label className="inline-flex items-center">
          <input
            type="radio"
            name="mode"
            value="off"
            className="form-radio text-indigo-600"
            checked={currentMode === 'off'}
            onChange={handleModeChange}
          />
          <span className="ml-2">Off (Stop forcé)</span>
        </label>
      </div>

      {currentMode === 'manual' && (
        <div>
          <label htmlFor="triacOpening" className="block text-sm font-medium text-gray-700 mb-2">
            Pourcentage d'ouverture du triac : {triacOpening}%
          </label>
          <input
            id="triacOpening"
            name="triacOpening"
            type="range"
            min="0"
            max="100"
            value={triacOpening}
            onChange={(e) => setTriacOpening(parseInt((e.target as HTMLInputElement).value))}
            className="w-full h-2 bg-gray-200 rounded-lg appearance-none cursor-pointer accent-indigo-600"
          />
          <div className="flex justify-between text-xs text-gray-500 mt-1">
            <span>0%</span>
            <span>50%</span>
            <span>100%</span>
          </div>
        </div>
      )}

      {currentMode === 'auto' && (
        <>
          <label className="block text-sm font-medium text-gray-700">Périodes de chauffe</label>
          <div className="space-y-4">
            {periods.map((period) => (
              <Period
                key={period.id}
                periodStart={period.start}
                periodEnd={period.end}
                sunRise={sunRiseMinutes} // This should probably come from props/context
                sunSet={sunSetMinutes} // This should probably come from props/context
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
          </div>
        </>
      )}

      <div className="space-x-4">
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