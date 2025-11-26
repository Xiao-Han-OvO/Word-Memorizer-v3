/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{vue,js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        'card-bg-color': 'var(--card-bg-color, #ffffff)',
        'border-color': 'var(--border-color, #e5e7eb)',
      },
      backgroundColor: {
        'card-bg': 'var(--card-bg-color, #ffffff)',
      },
      borderColor: {
        'border': 'var(--border-color, #e5e7eb)',
      },
      boxShadow: {
        'card': '0 4px 6px -1px rgba(0, 0, 0, 0.1), 0 2px 4px -1px rgba(0, 0, 0, 0.06)',
        'card-hover': '0 20px 25px -5px rgba(0, 0, 0, 0.1), 0 10px 10px -5px rgba(0, 0, 0, 0.04)'
      }
    },
  },
  plugins: [],
}
