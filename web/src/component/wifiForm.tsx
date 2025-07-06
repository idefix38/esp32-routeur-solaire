import { useEffect, useRef } from 'preact/hooks';
import { Loader } from 'lucide-react';

interface WifiFormData {
  ssid: string;
  password: string;
}

interface WifiFormProps {
  onSubmit: (data: WifiFormData) => Promise<any>;
  loading?: boolean;
  initialValues?: WifiFormData | null;
}

export const WifiForm = ({
  onSubmit,
  loading = false,
  initialValues,
}: WifiFormProps) => {
  const ssidRef = useRef<HTMLInputElement>(null);
  const passwordRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    if (initialValues) {
      if (ssidRef.current) ssidRef.current.value = initialValues.ssid;
      if (passwordRef.current) passwordRef.current.value = initialValues.password;
    }
  }, [initialValues]);

  const handleSubmit = async (e: Event) => {
    e.preventDefault();
    const ssid = ssidRef.current?.value || '';
    const password = passwordRef.current?.value || '';
    await onSubmit({ ssid, password });
  };

  return (
    <form onSubmit={handleSubmit} className="space-y-6">
      <div>
        <label htmlFor="ssid" className="block  font-medium text-gray-700">
          Nom du reseau / SSID
        </label>
        <div className="mt-1">
          <input
            id="ssid"
            name="ssid"
            type="text"
            required
            ref={ssidRef}
            className="block w-full rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500 text-lg px-4 py-3"
          />
        </div>
      </div>

      <div>
        <label htmlFor="password" className="block font-medium text-gray-700">
          Mot de passe
        </label>
        <div className="mt-1">
          <input
            id="password"
            name="password"
            type="password"
            required
            ref={passwordRef}
            className="block w-full rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500 text-lg px-4 py-3"
          />
        </div>
      </div>

      <div>
        <button
          type="submit"
          className="flex w-full justify-center items-center rounded-md bg-indigo-600 px-3 py-2 text-sm font-semibold text-white shadow-sm hover:bg-indigo-500 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-600"
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
};