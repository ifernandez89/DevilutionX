#!/usr/bin/env python3
"""
🧪 TEST: OLLAMA LOCAL - VARIACIONES DE TEXTO
Verifica que Ollama responde correctamente y genera variaciones apropiadas
"""

import requests
import json
import time

# Configuración
OLLAMA_URL = "http://localhost:11434"
MODEL = "qwen2.5:3b-instruct"
TIMEOUT = 8.0  # 8 segundos técnico

# Diálogos de prueba (estilo Diablo 1)
TEST_DIALOGUES = [
    "Greetings, stranger.",
    "What can I do for you?",
    "The darkness grows.",
    "Stay awhile and listen.",
    "I sense a soul in search of answers.",
    "The sanctity of this place has been fouled.",
    "Beware, the evil is strong here.",
    "May the light protect you.",
    "The dead walk among us.",
    "This is no place for the living."
]

def check_ollama_running():
    """Verifica si Ollama está corriendo"""
    try:
        response = requests.get(f"{OLLAMA_URL}/api/tags", timeout=1)
        return response.status_code == 200
    except:
        return False

def check_model_available():
    """Verifica si qwen2.5:3b-instruct está disponible"""
    try:
        response = requests.get(f"{OLLAMA_URL}/api/tags", timeout=1)
        if response.status_code == 200:
            data = response.json()
            models = [m['name'] for m in data.get('models', [])]
            return MODEL in models or 'qwen2.5:3b-instruct' in models
        return False
    except:
        return False

def build_prompt(text, tone="neutral"):
    """Construye el prompt para Ollama (ULTRACORTO - evita bloqueo)"""
    # Prompt ULTRACORTO - NO modificar
    return f"""Rewrite the sentence with a very small change. Keep the same meaning and tone. Do not add new ideas.

Sentence: "{text}" """

def call_ollama(text, tone="neutral"):
    """Llama a Ollama para generar variación"""
    prompt = build_prompt(text, tone)
    
    payload = {
        "model": MODEL,
        "prompt": prompt,
        "stream": False,
        "options": {
            "temperature": 0.3,       # No tan bajo, evita freeze
            "top_p": 0.9,             # Más fluido
            "num_predict": 24,        # Bajo para respuesta rápida
            "repeat_penalty": 1.05,   # Suave, no recalcula
            "stop": ["\n", ".", "!", "?"]
        }
    }
    
    try:
        start_time = time.time()
        response = requests.post(
            f"{OLLAMA_URL}/api/generate",
            json=payload,
            timeout=TIMEOUT
        )
        latency = (time.time() - start_time) * 1000  # ms
        
        if response.status_code == 200:
            data = response.json()
            variation = data.get('response', '').strip()
            
            # ✂️ POST-PROCESSING OBLIGATORIO: Limpiar ruido estructural
            
            # 1. Remover labels (case insensitive)
            variation_lower = variation.lower()
            if variation_lower.startswith('sentence:'):
                variation = variation[9:].strip()
            elif variation_lower.startswith('text:'):
                variation = variation[5:].strip()
            elif variation_lower.startswith('output:'):
                variation = variation[7:].strip()
            
            # 2. Limpiar comillas extras al inicio/final
            variation = variation.strip('"\'')
            
            # 3. Trim espacios
            variation = variation.strip()
            
            return variation, latency, None
        else:
            return None, latency, f"HTTP {response.status_code}"
    except requests.Timeout:
        return None, TIMEOUT * 1000, "Timeout"
    except Exception as e:
        return None, 0, str(e)

def is_lore_safe(original, variation):
    """Verifica que la variación sea lore-safe (MUY PERMISIVO)"""
    # Palabras prohibidas (pronombres modernos, asistente, términos meta)
    banned_words = {
        'i', 'you', 'your', 'my', 'assistant', 'ai', 'help', 'sorry', 
        'cannot', "can't", 'rewrite', 'rewritten', 'sentence', 'text', 
        'words', 'language', 'output', 'rules', 'label'
    }
    
    # Verificar palabras prohibidas
    variation_words = set(w.lower().strip('.,!?;:') for w in variation.split())
    for word in variation_words:
        if word in banned_words:
            return False, f"banned:{word}"
    
    # Palabras comunes permitidas (stopwords)
    common_words = {
        'a', 'an', 'the', 'is', 'are', 'was', 'were', 'be', 'been', 'being',
        'have', 'has', 'had', 'do', 'does', 'did', 'will', 'would', 'could',
        'should', 'may', 'might', 'must', 'can', 'of', 'to', 'in', 'for',
        'on', 'at', 'by', 'with', 'from', 'as', 'into', 'through', 'during',
        'before', 'after', 'above', 'below', 'between', 'under', 'again',
        'further', 'then', 'once', 'here', 'there', 'when', 'where', 'why',
        'how', 'all', 'both', 'each', 'few', 'more', 'most', 'other', 'some',
        'such', 'no', 'nor', 'not', 'only', 'own', 'same', 'so', 'than',
        'too', 'very', 'hmm', 'ah', 'oh', 'eh', 'uh', 'huh',
        'aye', 'nay', 'thee', 'thy', 'thou', 'hath', 'doth'
    }
    
    # Extraer palabras del original
    original_words = set(w.lower().strip('.,!?;:') for w in original.split())
    
    # Contar palabras completamente nuevas (MUY PERMISIVO - permitir hasta 40%)
    new_word_count = 0
    total_words = 0
    
    for word in variation_words:
        if word:
            total_words += 1
            # Solo contar como "nueva" si NO está en base Y NO es stopword
            if word not in original_words and word not in common_words:
                new_word_count += 1
    
    if total_words > 0 and (new_word_count / total_words) > 0.4:
        return False, f"too_many_new:{new_word_count}/{total_words}"
    
    return True, None

def check_length(original, variation, max_multiplier=1.3):
    """Verifica que la variación no sea demasiado larga (30% más permisivo)"""
    max_length = len(original) * max_multiplier
    return len(variation) <= max_length

def main():
    print("=" * 70)
    print("🧪 OLLAMA LOCAL - TEST DE VARIACIONES")
    print("=" * 70)
    
    # 1. Verificar Ollama
    print("\n1️⃣ Verificando Ollama...")
    if not check_ollama_running():
        print("❌ Ollama no está corriendo")
        print("   Ejecutá: ollama serve")
        return False
    print("✅ Ollama está corriendo")
    
    # 2. Verificar modelo
    print("\n2️⃣ Verificando modelo qwen2.5:3b-instruct...")
    if not check_model_available():
        print("❌ Modelo qwen2.5:3b-instruct no disponible")
        print("   Ejecutá: ollama pull qwen2.5:3b-instruct")
        return False
    print("✅ Modelo qwen2.5:3b-instruct disponible")
    
    # 3. Probar variaciones
    print("\n3️⃣ Probando variaciones...")
    print("-" * 70)
    
    results = {
        'total': 0,
        'success': 0,
        'failed': 0,
        'lore_safe': 0,
        'lore_unsafe': 0,
        'length_ok': 0,
        'length_fail': 0,
        'latencies': []
    }
    
    for i, dialogue in enumerate(TEST_DIALOGUES, 1):
        print(f"\n[{i}/{len(TEST_DIALOGUES)}] Original: \"{dialogue}\"")
        
        variation, latency, error = call_ollama(dialogue)
        results['total'] += 1
        
        if error:
            print(f"    ❌ Error: {error}")
            results['failed'] += 1
            continue
        
        if not variation:
            print(f"    ❌ Sin respuesta")
            results['failed'] += 1
            continue
        
        results['success'] += 1
        results['latencies'].append(latency)
        
        print(f"    ✅ Variación: \"{variation}\"")
        print(f"    ⏱️  Latencia: {latency:.0f}ms")
        
        # Verificar lore-safe
        is_safe, bad_word = is_lore_safe(dialogue, variation)
        if is_safe:
            print(f"    ✅ Lore-safe")
            results['lore_safe'] += 1
        else:
            print(f"    ❌ Lore-unsafe (palabra nueva: '{bad_word}')")
            results['lore_unsafe'] += 1
        
        # Verificar longitud
        if check_length(dialogue, variation):
            print(f"    ✅ Longitud OK ({len(variation)}/{int(len(dialogue) * 1.2)} max)")
            results['length_ok'] += 1
        else:
            print(f"    ❌ Demasiado largo ({len(variation)}/{int(len(dialogue) * 1.2)} max)")
            results['length_fail'] += 1
    
    # 4. Resumen
    print("\n" + "=" * 70)
    print("📊 RESUMEN DE RESULTADOS")
    print("=" * 70)
    
    print(f"\n🎯 Requests:")
    print(f"   Total: {results['total']}")
    print(f"   Exitosos: {results['success']} ({results['success']/results['total']*100:.1f}%)")
    print(f"   Fallidos: {results['failed']} ({results['failed']/results['total']*100:.1f}%)")
    
    if results['latencies']:
        avg_latency = sum(results['latencies']) / len(results['latencies'])
        min_latency = min(results['latencies'])
        max_latency = max(results['latencies'])
        print(f"\n⏱️  Latencia:")
        print(f"   Promedio: {avg_latency:.0f}ms")
        print(f"   Mínima: {min_latency:.0f}ms")
        print(f"   Máxima: {max_latency:.0f}ms")
    
    if results['success'] > 0:
        print(f"\n🛡️  Lore-Safe:")
        print(f"   Seguras: {results['lore_safe']} ({results['lore_safe']/results['success']*100:.1f}%)")
        print(f"   Inseguras: {results['lore_unsafe']} ({results['lore_unsafe']/results['success']*100:.1f}%)")
        
        print(f"\n📏 Longitud:")
        print(f"   OK: {results['length_ok']} ({results['length_ok']/results['success']*100:.1f}%)")
        print(f"   Excedida: {results['length_fail']} ({results['length_fail']/results['success']*100:.1f}%)")
    
    # 5. Evaluación final
    print("\n" + "=" * 70)
    success_rate = results['success'] / results['total'] if results['total'] > 0 else 0
    lore_safe_rate = results['lore_safe'] / results['success'] if results['success'] > 0 else 0
    
    if success_rate >= 0.8 and lore_safe_rate >= 0.7:
        print("✅ SISTEMA LISTO PARA USAR")
        print("   - Tasa de éxito aceptable")
        print("   - Variaciones lore-safe")
        print("   - Latencia dentro del timeout")
    elif success_rate >= 0.5:
        print("⚠️  SISTEMA FUNCIONAL PERO MEJORABLE")
        print("   - Algunas variaciones fallan")
        print("   - Considerar ajustar prompt o temperatura")
    else:
        print("❌ SISTEMA NECESITA AJUSTES")
        print("   - Demasiados fallos")
        print("   - Revisar configuración de Ollama")
    
    print("=" * 70)
    
    return success_rate >= 0.5

if __name__ == "__main__":
    import sys
    success = main()
    sys.exit(0 if success else 1)
