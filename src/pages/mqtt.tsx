import { pagePros } from '../app';
import { MqttForm } from '../component/mqttForm';

export default function MqttPage(props : pagePros ) {
  const handleSubmit = (data: { broker: string; user: string; password: string; topic: string }) => {
    console.log('Form submitted:', data);
    // Logique de traitement des données du formulaire
  };

  return (    
    <div className="container p-8">
          <div className="divide-y divide-gray-200 overflow-hidden rounded-lg bg-white shadow">
            <div className="px-4 py-5 sm:px-6 bg-indigo-600">
              <h1 className="text-xl font-semibold text-white">Paramètres Mqtt</h1>
            </div>
            <div className="px-4 py-5 sm:p-6 bg-gray-100">
              <MqttForm onSubmit={handleSubmit} />
            </div>
          </div>
    </div>
  );
}
