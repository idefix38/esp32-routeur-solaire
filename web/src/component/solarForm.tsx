import { useEffect, useRef } from "preact/hooks";
import { boilerConfig, shellyEmConfig } from "../context/configurationContext";
import { Loader } from "lucide-react";


interface solarFormProps {
  onSubmit: (boilerSettings?: boilerConfig, shellyEmSettings? : shellyEmConfig  ) => Promise<any>;
  loading?: boolean;
  boilerSettings?: boilerConfig;
  shellyEmSettings? : shellyEmConfig;  
}


/**
 * Simple form for updating the ShellyEM IP address.
 */
function SolarForm({ onSubmit, loading, boilerSettings, shellyEmSettings }: solarFormProps) {
  const ipRef = useRef<HTMLInputElement>(null);
  const channel1Ref = useRef<HTMLInputElement>(null);
  const channel2Ref = useRef<HTMLInputElement>(null);
  const modeAutoRef = useRef<HTMLInputElement>(null);
  const modeOnRef = useRef<HTMLInputElement>(null);
  const modeOffRef = useRef<HTMLInputElement>(null);
  //const [mode, setMode] = useState(boilerSettings?.mode || 'Auto');

  // Populate the form fields with initial values if provided
  useEffect(() => {
    console.log("Paramètre solaire :" , boilerSettings, shellyEmSettings);
    if (boilerSettings && shellyEmSettings) {      
      if (ipRef.current) ipRef.current.value = shellyEmSettings.ip || '';
      if (channel1Ref.current) channel1Ref.current.checked = shellyEmSettings.channel === '1';
      if (channel2Ref.current) channel2Ref.current.checked = shellyEmSettings.channel === '2';      
      if (modeAutoRef.current) modeAutoRef.current.checked = boilerSettings.mode === 'Auto';
      else if (modeOnRef.current) modeOnRef.current.checked = boilerSettings.mode === 'On';
      else if (modeOffRef.current) modeOffRef.current.checked = boilerSettings.mode === 'Off';
    }
  }, [boilerSettings, shellyEmSettings]);

  // Handles the form submission for Solar settings.
  const handleSubmit = (e: Event) => {
    e.preventDefault();
    const  dataShelly : shellyEmConfig = { 
      ip : ipRef.current?.value || '',
      channel : channel1Ref.current?.checked ? '1' : '2',      
    }
    const dataBoiler : boilerConfig = {
      mode : modeAutoRef.current?.checked ? 'Auto' : modeOnRef.current?.checked ? 'On' : 'Off',
    }    
    onSubmit(dataBoiler, dataShelly);
  };

  return (
    <form onSubmit={handleSubmit} className="space-y-6">
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
            defaultChecked={shellyEmSettings?.channel === '1'}
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
            defaultChecked={shellyEmSettings?.channel=== '2'}
          />
          <span className="ml-2">Channel 2</span>
        </label>
      </div>
      {/* // Mode */}
      <div>
  <span className="block text-sm font-medium text-gray-700 mb-1">Mode</span>
  <label className="inline-flex items-center mr-4">
    <input
      type="radio"
      name="mode"
      value="Auto"
      ref={modeAutoRef}
      className="form-radio text-indigo-600"
      defaultChecked={boilerSettings?.mode === 'Auto'}      
    />
    <span className="ml-2">Auto (Routeur Solaire)</span>
  </label>
  <label className="inline-flex items-center mr-4">
    <input
      type="radio"
      name="mode"
      value="On"
      ref={modeOnRef}
      className="form-radio text-indigo-600"
      defaultChecked={boilerSettings?.mode === 'On'}      
    />
    <span className="ml-2">On (Marche forcé)</span>
  </label>
  <label className="inline-flex items-center">
    <input
      type="radio"
      name="mode"
      value="Off"
      ref={modeOffRef}
      className="form-radio text-indigo-600"
      defaultChecked={boilerSettings?.mode === 'Off'}      
    />
    <span className="ml-2">Off</span>
  </label>
</div>
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

export default SolarForm;