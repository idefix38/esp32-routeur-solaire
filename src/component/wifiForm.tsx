import { useState } from 'preact/hooks';

interface WifiFormData {
  ssid: string;
  password: string;
}

interface WifiFormProps {
  onSubmit: (data: WifiFormData) => void;
}

export const WifiForm = ({ onSubmit }: WifiFormProps) => {
  const [formData, setFormData] = useState<WifiFormData>({
    ssid: '',
    password: '',
  });

  const handleSubmit = (e: Event) => {
    e.preventDefault();
    onSubmit(formData);
  };

  const handleChange = (e: Event) => {
    const target = e.target as HTMLInputElement;
    setFormData({
      ...formData,
      [target.name]: target.value,
    });
  };

  return (
    <form onSubmit={handleSubmit} className="space-y-6">
      <div>
        <label htmlFor="ssid" className="block text-sm font-medium text-gray-700">
          Nom du reseau / SSID
        </label>
        <div className="mt-1">
          <input
            id="ssid"
            name="ssid"
            type="text"
            required
            value={formData.ssid}
            onChange={handleChange}
            className="block w-full rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500 text-lg px-4 py-3"
          />
        </div>
      </div>

      <div>
        <label htmlFor="password" className="block text-sm font-medium text-gray-700">
          Mot de passe
        </label>
        <div className="mt-1">
          <input
            id="password"
            name="password"
            type="password"
            required
            value={formData.password}
            onChange={handleChange}
            className="block w-full rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500 text-lg px-4 py-3"
          />
        </div>
      </div>

      <div>
        <button
          type="submit"
          className="flex w-full justify-center rounded-md bg-indigo-600 px-3 py-2 text-sm font-semibold text-white shadow-sm hover:bg-indigo-500 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-600"
        >
          Enregistrer
        </button>
      </div>
    </form>
  );
};