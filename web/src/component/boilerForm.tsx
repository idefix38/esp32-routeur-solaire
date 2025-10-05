import { Loader } from "lucide-react";
import { boilerConfig } from "../context/configurationContext";
import { useRef, useEffect, useState } from "preact/hooks";
import { FormEvent } from "react";
import { Period } from "./period";


interface boilerFormProps {
  onSubmit: (boilerSettings?: boilerConfig) => Promise<any>;
  loading?: boolean;
  boilerSettings?: boilerConfig;
}

export const BoilerForm = ({ onSubmit, boilerSettings, loading }: boilerFormProps) => {

  const temperatureRef = useRef<HTMLInputElement>(null);
  const [periodStart, setPeriodStart] = useState(6*60);
  const [periodEnd, setPeriodEnd] = useState(18*60);

  // Affichege des valeurs par défaut
  useEffect(() => {
    if (boilerSettings) {
      if (temperatureRef.current) {
        temperatureRef.current.value = boilerSettings.temperature?.toString() || '50';
      }
      setPeriodStart(boilerSettings.periodStart || 6*60);
      setPeriodEnd(boilerSettings.periodEnd || 18*60);
    }
  }, [boilerSettings]);

  // Donnée à envoyer
  const handleSubmit = (e: FormEvent<HTMLFormElement>) => {
    e.preventDefault();
    if (temperatureRef.current && boilerSettings) {
      const newSettings: boilerConfig = {
        mode: boilerSettings.mode,
        temperature: parseFloat(temperatureRef.current.value),
        periodStart: periodStart,
        periodEnd: periodEnd
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
      <Period periodStart={periodStart} periodEnd={periodEnd} sunRise={6*60} sunSet={18*60} onDelete={() => {console.log('delete')}} />  
      <Period periodStart={periodStart} periodEnd={periodEnd} sunRise={6*60} sunSet={18*60} canDelete={true}  onDelete={() => {console.log('delete')}} />  
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
    </form>
  );
}