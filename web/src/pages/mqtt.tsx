
import { pagePros } from '../app';
import { MqttForm } from '../component/mqttForm';
import { useToast } from '../context/ToastContext';
import { useEsp32Api } from '../hooks/useEsp32Api';
import { mqttConfig, useConfig } from '../context/configurationContext';

/**
 * Page for configuring MQTT settings.
 * It fetches the current MQTT settings from the ESP32 device and allows the user to update them.
 * The form includes fields for broker, port, username, password, and topic, and displays a success or error message upon submission.
 * @param props 
 * @returns 
 */
export default function MqttPage(props: pagePros) {
  const { setToast } = useToast();
  const { callApi, loading } = useEsp32Api();
  const config = useConfig();
  

  // useEffect(() => {
  //   (async () => {
  //     const result = await callApi('/getConfig');
  //     if (result.success && result.data?.mqtt) {
  //       setInitialValues({
  //         broker: result.data.mqtt.server || '',
  //         port: result.data.mqtt.port || 1883,
  //         user: result.data.mqtt.username || '',
  //         password: result.data.mqtt.password || '',
  //         topic: result.data.mqtt.topic || ''
  //       });
  //     }
  //   })();
  // }, []);

  /**
   * Handles the form submission for MQTT settings.
   * @param data - The form data containing broker, user, password, topic, and port.
   */
  const handleSubmit = async (data: mqttConfig) => {
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
          <div className={(loading) ? 'pointer-events-none opacity-50 relative' : ''}>
            <MqttForm onSubmit={handleSubmit} initialValues={config.value?.mqtt} loading={loading} />
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
}
