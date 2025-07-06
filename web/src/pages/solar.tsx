import { useEffect, useState, useRef } from 'preact/hooks';
import { pagePros } from '../app';
import { useToast } from '../context/ToastContext';
import { useEsp32Api } from '../hooks/useEsp32Api';

/**
 * Page for configuring Solar Router settings.
 * It fetches the current ShellyEM IP address from the ESP32 device and allows the user to update it.
 * The form includes a field for the IP address and displays a success or error message upon submission.
 */
export default function SolarPage(props: pagePros) {
  const { setToast } = useToast();
  const { callApi, loading } = useEsp32Api();
  const [initialValues, setInitialValues] = useState<{ shellyEmIp: string, shellyEmChannel?: string } | null>(null);

  // Fetch the current ShellyEM IP address and channel from the ESP32 device
  useEffect(() => {
    (async () => {
      const result = await callApi('/getConfig');
     
      if (result.success && result.data?.shelly) {
        setInitialValues({
          shellyEmIp: result.data.shelly.ip || '',
          shellyEmChannel: result.data.shelly.channel || '1',
        });
      }
    })();
  }, []);

  // Handles the form submission for Solar settings.
  const handleSolarSubmit = async (data: { shellyEmIp: string, shellyEmChannel: string }) => {
    const result = await callApi('/saveSolarSettings', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(data)
    });
    if (result.success) {
      setToast({message: 'Adresse IP ShellyEM enregistrée avec succès', type: 'success'});
    } else {
      setToast({message: "Erreur lors de l'enregistrement de l'adresse IP ShellyEM", type: 'error'});
    }
  };

  return (
    <div className="container p-8">
      <div className="divide-y divide-gray-200 overflow-hidden rounded-lg bg-white shadow">
        <div className="px-4 py-5 sm:px-6 bg-indigo-600">
          <h1 className="text-xl font-semibold text-white">Paramètres Routeur Solaire</h1>
        </div>
        <div className="px-4 py-5 sm:p-6 bg-gray-100">
          <div className={(!initialValues || loading) ? 'pointer-events-none opacity-50 relative' : ''}>
            <SolarForm onSubmit={handleSolarSubmit} loading={loading} initialValues={initialValues} />
            {(!initialValues || loading) && (
              <div className="absolute inset-0 flex items-center justify-center z-10">
                <span className="text-indigo-600 font-semibold text-lg animate-pulse">Chargement...</span>
              </div>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}

/**
 * Simple form for updating the ShellyEM IP address.
 */
function SolarForm({ onSubmit, loading, initialValues }: {
  onSubmit: (data: { shellyEmIp: string, shellyEmChannel: string }) => void;
  loading: boolean;
  initialValues: { shellyEmIp: string, shellyEmChannel?: string } | null;
}) {
  const ipRef = useRef<HTMLInputElement>(null);
  const channel1Ref = useRef<HTMLInputElement>(null);
  const channel2Ref = useRef<HTMLInputElement>(null);

    // Populate the form fields with initial values if provided
  useEffect(() => {
    if (initialValues) {
      if (ipRef.current) ipRef.current.value = initialValues.shellyEmIp;
      if (channel1Ref.current) channel1Ref.current.checked = initialValues.shellyEmChannel === '1';
      if (channel2Ref.current) channel2Ref.current.checked = initialValues.shellyEmChannel === '2';
    }
  }, [initialValues]);

  // Handles the form submission for Solar settings.
  const handleSubmit = (e: Event) => {
    e.preventDefault();
    const shellyEmIp = ipRef.current?.value || '';
    const shellyEmChannel = channel1Ref.current?.checked ? '1' : '2';
    onSubmit({ shellyEmIp, shellyEmChannel });
  };

  return (
    <form onSubmit={handleSubmit} className="space-y-4">
      <div>
        <label htmlFor="shellyIp" className="block text-sm font-medium text-gray-700">Adresse IP du module ShellyEM</label>
        <input
          id="shellyIp"
          name="shellyIp"
          type="text"
          ref={ipRef}
          className="mt-1 block w-full rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500 text-lg px-4 py-3"
          placeholder="192.168.1.100"
          required
        />
      </div>
      <div>
        <span className="block text-sm font-medium text-gray-700 mb-1">Channel du module Shelly</span>
        <label className="inline-flex items-center mr-4">
          <input
            type="radio"
            name="shellyChannel"
            value="1"
            ref={channel1Ref}
            className="form-radio text-indigo-600"
            defaultChecked={initialValues?.shellyEmChannel === '1'}
          />
          <span className="ml-2">Channel 1</span>
        </label>
        <label className="inline-flex items-center">
          <input
            type="radio"
            name="shellyChannel"
            value="2"
            ref={channel2Ref}
            className="form-radio text-indigo-600"
            defaultChecked={initialValues?.shellyEmChannel === '2'}
          />
          <span className="ml-2">Channel 2</span>
        </label>
      </div>
      <button
        type="submit"
        className="inline-flex justify-center rounded-md border border-transparent bg-indigo-600 py-2 px-4 text-sm font-medium text-white shadow-sm hover:bg-indigo-700 focus:outline-none focus:ring-2 focus:ring-indigo-500 focus:ring-offset-2"
        disabled={loading}
      >
        {loading ? 'Enregistrement...' : 'Enregistrer'}
      </button>
    </form>
  );
}
