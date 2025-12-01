/**
 * API Client - Axios instance with default configuration
 */

import axios from 'axios'
import { SERVER_URL } from '../constants'

// Create axios instance
const api = axios.create({
  baseURL: SERVER_URL,
  timeout: 10000,
  headers: {
    'Content-Type': 'application/json'
  }
})

// Request interceptor - add session ID
api.interceptors.request.use(
  (config) => {
    const sessionId = localStorage.getItem('sessionId')
    if (sessionId) {
      config.headers['X-Session-ID'] = sessionId
    }
    return config
  },
  (error) => {
    return Promise.reject(error)
  }
)

// Response interceptor - handle errors
api.interceptors.response.use(
  (response) => response,
  (error) => {
    return Promise.reject(error)
  }
)

export default api
