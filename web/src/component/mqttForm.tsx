import { useState } from 'preact/hooks';

interface MqttFormData {
  broker: string;
  port: number;
  user: string;
  password: string;
  topic: string;
}

interface MqttFormProps {
  onSubmit: (data: MqttFormData) => void;
}

export const MqttForm = ({ onSubmit }: MqttFormProps) => {
  const [formData, setFormData] = useState<MqttFormData>({
    broker: '',
    port: 1883,
    user: '',
    password: '',
    topic: ''
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
        <label htmlFor="broker" className="block text-sm font-medium text-gray-700">
          Broker Mqtt
        </label>
        <div className="mt-1">
          <input
            id="broker"
            name="broker"
            type="text"
            required
            value={formData.broker}
            onChange={handleChange}
            className="block w-full rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500 text-lg px-4 py-3"
          />
        </div>
      </div>
      <div>
        <label htmlFor="broker" className="block font-medium text-gray-700">
          Mqtt port
        </label>
        <div className="mt-1">
          <input
            id="port"
            name="port"
            type="number"
            required
            value={formData.port}
            onChange={handleChange}
            className="block w-full rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500 text-lg px-4 py-3"
          />
        </div>
      </div>
      <div>
        <label htmlFor="user" className="block font-medium text-gray-700">
          User
        </label>
        <div className="mt-1">
          <input
            id="user"
            name="user"
            type="text"
            required
            value={formData.user}
            onChange={handleChange}
            className="block w-full rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500 text-lg px-4 py-3"
          />
        </div>
      </div>
      <div>
        <label htmlFor="password" className="block font-medium text-gray-700">
          Password
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
        <label htmlFor="topic" className="block font-medium text-gray-700">
          Mqtt Topic
        </label>
        <div className="mt-1">
          <input
            id="topic"
            name="topic"
            type="text"
            required
            value={formData.topic}
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