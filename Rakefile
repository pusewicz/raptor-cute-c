# frozen_string_literals: true

def build_type
  ENV.fetch("BUILD_TYPE", "Debug").tap do |type|
    unless VALID_BUILD_TYPES.include?(type)
      abort "Invalid BUILD_TYPE: #{type}, expected one of #{VALID_BUILD_TYPESS}"
    end
  end
end

def build_dir
  File.join(__dir__, 'build', build_type.downcase)
end

VALID_BUILD_TYPES = %w[Debug Release].freeze
PWD = File.expand_path(__dir__)
PID_FILE = File.join(build_dir, 'raptor.pid')
EXE_FILE = File.join(build_dir, 'Raptor') # For development

directory build_dir do
  sh "cmake -S #{PWD} -B #{build_dir} -G Ninja -DCMAKE_BUILD_TYPE=#{build_type}"
end

desc "Set mode to release"
task :release do
  ENV["BUILD_TYPE"] = "Release"
end

desc 'Compile the project'
task compile: build_dir do
  sh "cmake --build #{build_dir} --parallel"
end

def run(cmd)
  puts "Executing: #{cmd}"
  pid = Process.spawn(cmd, chdir: build_dir)
  File.write(PID_FILE, pid)
  puts "Started process #{pid} with PID file at #{PID_FILE}"
ensure
  Process.wait(pid)
  File.delete(PID_FILE) if File.exist?(PID_FILE)
end

desc 'Run the project'
task run: :compile do
  if File.exist?(EXE_FILE)
    run EXE_FILE
  else
    abort "#{EXE_FILE} does not exist, aborting..."
  end
end

namespace :run do
  desc 'Run the project with debugger attached'
  task :debugger do
    run "lldb #{EXE_FILE} -o run"
  end
end

desc 'Watch for changes and recompile'
task :watch do
  require 'listen'
  listener = Listen.to('src', 'include') do |_modified, _added, _removed|
    if File.exist?(PID_FILE)
      pid = File.read(PID_FILE).to_i

      begin
        Process.kill(0, pid)
        Rake::Task[:compile].execute
        puts "Sending SIGHUP to process #{pid}"
        Process.kill('SIGHUP', pid)
      rescue Errno::ESRCH
        warn "Process #{pid} not found"
      end
    else
      warn "No PID file found at #{PID_FILE}, not sending SIGHUP"
    end
  end
  puts 'Listening for changes...'
  listener.start
  sleep
end

namespace :headers do
  desc 'Update header files'
  task :update do
    {
      'include/pico_ecs.h' => 'https://raw.githubusercontent.com/empyreanx/pico_headers/refs/heads/main/pico_ecs.h'
    }.each do |local_path, url|
      sh "curl -o #{local_path} #{url}"
    end
  end
end

desc "Formats the source code using clang-format"
task :format do
  sh %(fd --full-path src/ -e "c" -e "h" | xargs clang-format -i)
end

task default: :compile
