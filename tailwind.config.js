/** @type {import('tailwindcss').Config} */
export default {
  content: [
    './web/index.html',
    './web/src/**/*.{js,ts,jsx,tsx}',
  ],
  theme: {
    extend: {
      colors: {
        'custom-orange': '#FFC107',
      },
    },
    container: {
      center: true,
      padding: '2rem',
    },
  },
  plugins: [],
}
