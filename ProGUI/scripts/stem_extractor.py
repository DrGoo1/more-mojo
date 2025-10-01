#!/usr/bin/env python3
"""
Stem Extractor for Steal The Mojo
Uses Demucs for high-quality AI-based source separation
"""

import sys
import json
import os

def main():
    if len(sys.argv) < 4:
        print(json.dumps({
            'success': False,
            'error': 'Usage: stem_extractor.py <input_file> <stem_type> <output_file>'
        }))
        sys.exit(1)
    
    input_path = sys.argv[1]
    stem_type = sys.argv[2]  # 'vocals', 'drums', 'bass', 'other'
    output_path = sys.argv[3]
    
    try:
        # Try to import Demucs
        try:
            import torch
            import torchaudio
            from demucs.pretrained import get_model
            from demucs.apply import apply_model
        except ImportError as e:
            print(json.dumps({
                'success': False,
                'error': f'Demucs dependencies not installed: {str(e)}\n' +
                        'Please install with: pip install demucs torch torchaudio'
            }))
            sys.exit(1)
        
        # Check if input file exists
        if not os.path.exists(input_path):
            print(json.dumps({
                'success': False,
                'error': f'Input file not found: {input_path}'
            }))
            sys.exit(1)
        
        # Load Demucs model (htdemucs is the best quality v4 model)
        print(json.dumps({
            'status': 'loading_model',
            'message': 'Loading Demucs AI model...'
        }), file=sys.stderr)
        
        model = get_model('htdemucs')
        model.eval()
        
        # Load audio file
        print(json.dumps({
            'status': 'loading_audio',
            'message': 'Loading audio file...'
        }), file=sys.stderr)
        
        audio, sr = torchaudio.load(input_path)
        
        # Ensure stereo
        if audio.shape[0] == 1:
            audio = audio.repeat(2, 1)
        
        # Resample to model's expected rate if needed (44.1kHz)
        if sr != model.samplerate:
            print(json.dumps({
                'status': 'resampling',
                'message': f'Resampling from {sr}Hz to {model.samplerate}Hz...'
            }), file=sys.stderr)
            
            resampler = torchaudio.transforms.Resample(sr, model.samplerate)
            audio = resampler(audio)
            sr = model.samplerate
        
        # Run separation
        print(json.dumps({
            'status': 'separating',
            'message': f'Extracting {stem_type} stem (this may take a minute)...'
        }), file=sys.stderr)
        
        with torch.no_grad():
            # Add batch dimension
            audio_batch = audio[None]
            
            # Apply model
            stems = apply_model(model, audio_batch)
            
            # Remove batch dimension
            stems = stems[0]
        
        # Extract requested stem
        # Demucs v4 order: drums, bass, other, vocals
        stem_indices = {
            'drums': 0,
            'bass': 1,
            'other': 2,
            'vocals': 3
        }
        
        if stem_type not in stem_indices:
            print(json.dumps({
                'success': False,
                'error': f'Invalid stem type: {stem_type}. Must be one of: {list(stem_indices.keys())}'
            }))
            sys.exit(1)
        
        target_stem = stems[stem_indices[stem_type]]
        
        # Save output
        print(json.dumps({
            'status': 'saving',
            'message': 'Saving extracted stem...'
        }), file=sys.stderr)
        
        torchaudio.save(output_path, target_stem, sr)
        
        # Return success
        result = {
            'success': True,
            'path': output_path,
            'samplerate': sr,
            'stem_type': stem_type,
            'channels': target_stem.shape[0],
            'samples': target_stem.shape[1]
        }
        
        print(json.dumps(result))
        sys.exit(0)
        
    except Exception as e:
        import traceback
        print(json.dumps({
            'success': False,
            'error': str(e),
            'traceback': traceback.format_exc()
        }))
        sys.exit(1)

if __name__ == '__main__':
    main()
