
import { pagePros } from '../app';
import { BoilerForm } from '../component/boilerForm';
import { boilerConfig, useConfig } from '../context/configurationContext';
import { useToast } from '../context/ToastContext';
import { useEsp32Api } from '../hooks/useEsp32Api';

export default function SolarPage(props: pagePros) {

    const { setToast } = useToast();
    const { callApi, loading } = useEsp32Api();
    const config = useConfig();



  // Handles the form submission for Solar settings.
    const handleSubmit = async (boilerSettings?: boilerConfig) => {
      const result = await callApi('/saveBoilerSettings', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({...boilerSettings})
      });
      if (result.success) {
        setToast({message: 'Paramètres du chauffe eau enregistrés avec succès', type: 'success'});
      } else {
        setToast({message: "Erreur lors de l'enregistrement des paramètres", type: 'error'});
      }
    };

  return (
    <div className="container p-8">
      <div className="divide-y divide-gray-200 overflow-hidden rounded-lg bg-white shadow">
        <div className="px-4 py-5 sm:px-6 bg-indigo-600">
          <h1 className="text-xl font-semibold text-white">Paramètres du chauffe eau</h1>
        </div>
        <div className="px-4 py-5 sm:p-6 bg-gray-100">
          <div className={(loading) ? 'pointer-events-none opacity-50 relative' : ''}>
            <BoilerForm onSubmit={handleSubmit} loading={loading} boilerSettings={config.value?.boiler}  sunRiseMinutes={config.value?.solar?.sunRiseMinutes ?? 6*60} sunSetMinutes={config.value?.solar?.sunSetMinutes ?? 18*60}  />
            {(loading) && (
              <div className="absolute inset-0 flex items-center justify-center z-10">
                <span className="text-indigo-600 font-semibold text-lg animate-pulse">Chargement...</span>
              </div>
            )}
          </div>
        </div>
      </div>
    </div>    
  );
};
