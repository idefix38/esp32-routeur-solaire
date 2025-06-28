import { useEffect, useState } from 'preact/hooks';
import { pagePros } from '../app';
import { MqttForm } from '../component/mqttForm';
import { useToast } from '../context/ToastContext';
import { useEsp32Api } from '../hooks/useEsp32Api';

export default function MqttPage(props: pagePros) {
  const { setToast } = useToast();
  const { callApi, loading } = useEsp32Api();
  const [initialValues, setInitialValues] = useState<any>(null);

  useEffect(() => {
    (async () => {
      const result = await callApi('/getConfig');
      if (result.success && result.data?.mqtt) {
        setInitialValues({
          broker: result.data.mqtt.server || '',
          port: result.data.mqtt.port || 1883,
          user: result.data.mqtt.username || '',
          password: result.data.mqtt.password || '',
          topic: result.data.mqtt.topic || ''
        });
      }
    })();
  }, []);

  const handleSubmit = async (data: { broker: string; user: string; password: string; topic: string; port: number }) => {
    const result = await callApi('/saveMqttSettings', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(data)
    });
    if (result.success) {
      setToast({ message: 'Paramètres MQTT enregistrés avec succès', type: 'success' });      
    } else {
      setToast({ message: 'Erreur lors de l\'enregistrement des paramètres MQTT', type: 'error' }); 
    }
  };

  return (    
    <div className="container p-8">
          <div className="divide-y divide-gray-200 overflow-hidden rounded-lg bg-white shadow">
            <div className="px-4 py-5 sm:px-6 bg-indigo-600">
              <h1 className="text-xl font-semibold text-white">Paramètres Mqtt</h1>
            </div>
            <div className="px-4 py-5 sm:p-6 bg-gray-100">
              <MqttForm onSubmit={handleSubmit} initialValues={initialValues}  loading={loading}/>
            </div>
          </div>
    </div>
  );
}
