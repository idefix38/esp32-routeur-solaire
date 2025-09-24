import { Loader } from 'lucide-react';
import { useEffect, useRef } from 'preact/hooks';
import { mqttConfig } from '../context/configurationContext';



interface MqttFormProps {
  onSubmit: (data: mqttConfig) => void;
  loading?: boolean;
  initialValues?: mqttConfig | null;
}

export const MqttForm = ({ onSubmit, initialValues, loading }: MqttFormProps) => {
  const brokerRef = useRef<HTMLInputElement>(null);
  const portRef = useRef<HTMLInputElement>(null);
  const userRef = useRef<HTMLInputElement>(null);
  const passwordRef = useRef<HTMLInputElement>(null);
  const topicRef = useRef<HTMLInputElement>(null);

  useEffect(() => {
    if (initialValues) {
      if (brokerRef.current) brokerRef.current.value = initialValues.server;
      if (portRef.current) portRef.current.value = String(initialValues.port);
      if (userRef.current) userRef.current.value = initialValues.username;      
      if (passwordRef.current) passwordRef.current.value = initialValues.password;
      if (topicRef.current) topicRef.current.value = initialValues.topic;
    }
  }, [initialValues]);

  const handleSubmit = (e: Event) => {
    e.preventDefault();
    onSubmit({
      server: brokerRef.current?.value || '',
      port: Number(portRef.current?.value) || 1883,
      username: userRef.current?.value || '',
      password: passwordRef.current?.value || '',
      topic: topicRef.current?.value || '',
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
            ref={brokerRef}
            className="block w-full rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500 text-lg px-4 py-3"
          />
        </div>
      </div>
      <div>
        <label htmlFor="port" className="block font-medium text-gray-700">
          Mqtt port
        </label>
        <div className="mt-1">
          <input
            id="port"
            name="port"
            type="number"
            required
            ref={portRef}
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
            ref={userRef}
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
            ref={passwordRef}
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
            ref={topicRef}
            className="block w-full rounded-md border-gray-300 shadow-sm focus:border-indigo-500 focus:ring-indigo-500 text-lg px-4 py-3"
          />
        </div>
      </div>
      <div>
        <button
          type="submit"
          className="flex w-full justify-center rounded-md bg-indigo-600 px-3 py-2 text-sm font-semibold text-white shadow-sm hover:bg-indigo-500 focus-visible:outline focus-visible:outline-2 focus-visible:outline-offset-2 focus-visible:outline-indigo-600"
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